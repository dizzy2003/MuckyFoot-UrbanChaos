// Copyright (c) 2017 FaultyRAM
//
// Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
// http://www.apache.org/licenses/LICENSE-2.0> or the MIT license <LICENSE-MIT
// or http://opensource.org/licenses/MIT>, at your option. This file may not be
// copied, modified, or distributed except according to those terms.

//! Configuration variables.

use openchaos_config;
use openchaos_config::Config;
use std::ffi::{CStr, CString};
use std::io;
use std::path::Path;
use std::ptr;
use std::sync::RwLock;
use super::types::{CBYTE, SLONG};
use toml::value::{Table, Value};

/// An error message for when accessing configuration data fails due to a poisoned `RwLock`.
const POISONED: &str = "`RwLock` is poisoned";

lazy_static! {
    /// A set of configuration data.
    static ref CONFIG: RwLock<Value> = RwLock::new(Value::Table(Table::new()));
}

/// Loads configuration data from a file into a `Table`.
fn table_from_file<P: AsRef<Path>>(path: P) -> io::Result<Value> {
    openchaos_config::from_file(path)
        .and_then(|c| {
            Value::try_from(c).map_err(|e| io::Error::new(io::ErrorKind::Other, e))
        })
        .and_then(|v| if v.is_table() {
            Ok(v)
        } else {
            Err(io::Error::new(
                io::ErrorKind::Other,
                "configuration file is not a TOML table",
            ))
        })
}

/// Writes configuration data from a `Table` to a file.
fn table_to_file<P: AsRef<Path>>(path: P) -> io::Result<()> {
    CONFIG
        .read()
        .expect(POISONED)
        .clone()
        .try_into::<Config>()
        .map_err(|e| io::Error::new(io::ErrorKind::Other, e))
        .and_then(|c| openchaos_config::to_file(&c, path))
}

#[no_mangle]
/// Loads configuration data from a file at the specified path.
pub unsafe extern "C" fn ENV_load(path: *const CBYTE) {
    let p_cstr = CStr::from_ptr(path);
    *CONFIG.write().expect(POISONED) = p_cstr
        .to_str()
        .map_err(|e| io::Error::new(io::ErrorKind::InvalidInput, e))
        .and_then(|s| table_from_file(s))
        .or_else(|_| Value::try_from(Config::default()))
        .expect("could not load configuration data");
}

#[no_mangle]
/// Writes configuration data to a file at the specified path.
pub unsafe extern "C" fn ENV_save(path: *const CBYTE) {
    let p_cstr = CStr::from_ptr(path);
    p_cstr
        .to_str()
        .map_err(|e| io::Error::new(io::ErrorKind::InvalidInput, e))
        .and_then(|s| table_to_file(s))
        .unwrap_or(())
}

#[no_mangle]
/// Returns a string copied from the configuration data, or `NULL` if it is not found.
pub unsafe extern "C" fn ENV_get_value_string(
    property: *const CBYTE,
    section: *const CBYTE,
) -> *mut CBYTE {
    let s_cstr = CStr::from_ptr(section);
    let p_cstr = CStr::from_ptr(property);
    s_cstr
        .to_str()
        .and_then(|s| p_cstr.to_str().map(|p| (s, p)))
        .map_err(|e| io::Error::new(io::ErrorKind::InvalidInput, e))
        .and_then(|(s, p)| {
            CONFIG
                .read()
                .expect(POISONED)
                .get(s)
                .and_then(|k| k.get(p))
                .ok_or(io::Error::new(
                    io::ErrorKind::Other,
                    "could not retrieve the specified property",
                ))
                .and_then(|v| {
                    v.clone().try_into::<CString>().map_err(|e| {
                        io::Error::new(io::ErrorKind::InvalidData, e)
                    })
                })
                .map(|cs| cs.into_raw())
        })
        .unwrap_or(ptr::null_mut())
}

#[no_mangle]
/// Returns an integer from the configuration data, or a user-specified default if it is not found.
pub unsafe extern "C" fn ENV_get_value_number(
    property: *const CBYTE,
    def: SLONG,
    section: *const CBYTE,
) -> SLONG {
    let s_cstr = CStr::from_ptr(section);
    let p_cstr = CStr::from_ptr(property);
    s_cstr
        .to_str()
        .and_then(|s| p_cstr.to_str().map(|p| (s, p)))
        .map_err(|e| io::Error::new(io::ErrorKind::InvalidInput, e))
        .and_then(|(s, p)| {
            CONFIG
                .read()
                .expect(POISONED)
                .get(s)
                .and_then(|k| k.get(p))
                .ok_or(io::Error::new(
                    io::ErrorKind::Other,
                    "could not retrieve the specified property",
                ))
                .and_then(|v| {
                    v.clone().try_into::<SLONG>().map_err(|e| {
                        io::Error::new(io::ErrorKind::InvalidData, e)
                    })
                })
        })
        .unwrap_or(def)
}

#[no_mangle]
/// Updates the value of a string within the configuration data.
pub unsafe extern "C" fn ENV_set_value_string(
    property: *const CBYTE,
    value: *const CBYTE,
    section: *const CBYTE,
) {
    let s_cstr = CStr::from_ptr(section);
    let p_cstr = CStr::from_ptr(property);
    let v_cstr = CStr::from_ptr(value);
    s_cstr
        .to_str()
        .and_then(|s| p_cstr.to_str().map(|p| (s, p)))
        .and_then(|(s, p)| v_cstr.to_str().map(|v| (s, p, v)))
        .map_err(|e| io::Error::new(io::ErrorKind::InvalidInput, e))
        .and_then(|(s, p, v)| {
            CONFIG
                .write()
                .expect(POISONED)
                .get_mut(s)
                .and_then(|k| k.get_mut(p))
                .map(|vl| *vl = Value::String(v.to_owned()))
                .ok_or(io::Error::new(
                    io::ErrorKind::Other,
                    "could not update the specified property",
                ))
        })
        .unwrap_or(())
}

#[no_mangle]
/// Updates the value of an integer within the configuration data.
pub unsafe extern "C" fn ENV_set_value_number(
    property: *const CBYTE,
    value: SLONG,
    section: *const CBYTE,
) {
    let s_cstr = CStr::from_ptr(section);
    let p_cstr = CStr::from_ptr(property);
    s_cstr
        .to_str()
        .and_then(|s| p_cstr.to_str().map(|p| (s, p)))
        .map_err(|e| io::Error::new(io::ErrorKind::InvalidInput, e))
        .and_then(|(s, p)| {
            CONFIG
                .write()
                .expect(POISONED)
                .get_mut(s)
                .and_then(|k| k.get_mut(p))
                .map(|v| *v = Value::Integer(value.into()))
                .ok_or(io::Error::new(
                    io::ErrorKind::Other,
                    "could not update the specified property",
                ))
        })
        .unwrap_or(())
}

#[no_mangle]
/// Destroys a string allocated by `ENV_get_value_string`.
pub unsafe extern "C" fn ENV_free_string(s: *mut CBYTE) {
    let _die = CString::from_raw(s);
}
