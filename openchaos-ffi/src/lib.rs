// Copyright (c) 2017 FaultyRAM
//
// Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
// http://www.apache.org/licenses/LICENSE-2.0> or the MIT license <LICENSE-MIT
// or http://opensource.org/licenses/MIT>, at your option. This file may not be
// copied, modified, or distributed except according to those terms.

//! Provides replacements for C functions in OpenChaos.

#![cfg_attr(feature = "clippy", forbid(clippy))]
#![cfg_attr(feature = "clippy", forbid(clippy_internal))]
#![cfg_attr(feature = "clippy", forbid(clippy_pedantic))]
#![cfg_attr(feature = "clippy", forbid(clippy_restrictions))]
#![deny(warnings)]
#![allow(bad_style)]
#![forbid(box_pointers)]
#![forbid(fat_ptr_transmutes)]
#![deny(missing_copy_implementations)]
#![forbid(missing_debug_implementations)]
#![forbid(missing_docs)]
#![forbid(trivial_casts)]
#![forbid(trivial_numeric_casts)]
#![forbid(unused_extern_crates)]
#![forbid(unused_import_braces)]
#![deny(unused_qualifications)]
#![forbid(unused_results)]
#![forbid(variant_size_differences)]

#[macro_use]
extern crate lazy_static;
extern crate openchaos_config;
extern crate toml;

pub mod env;

/// MFStdLib type aliases.
pub mod types {
    use std::os::raw::{c_char, c_long};

    /// The `CBYTE` type.
    pub type CBYTE = c_char;
    /// The `SLONG` type.
    pub type SLONG = c_long;
}
