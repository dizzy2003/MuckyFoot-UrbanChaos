// Copyright (c) 2017 FaultyRAM
//
// Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
// http://www.apache.org/licenses/LICENSE-2.0> or the MIT license <LICENSE-MIT
// or http://opensource.org/licenses/MIT>, at your option. This file may not be
// copied, modified, or distributed except according to those terms.

//! Configuration variables.

#if !defined(ENV_H)
#define ENV_H

#if !defined(C_API)
#if defined(__cplusplus)
#define C_API "C"
#else
#define C_API
#endif
#endif

/// Loads configuration data from a file at the specified path.
extern C_API void ENV_load(const CBYTE *path);

/// Writes configuration data to a file at the specified path.
extern C_API void ENV_save(const CBYTE *path);

/// Returns a string copied from the configuration data, or `NULL` if it is not found.
///
/// The returned string is allocated by this function and must be freed with
/// `ENV_free_string`.
extern C_API CBYTE *ENV_get_value_string(const CBYTE *property, const CBYTE *section);

/// Returns an integer from the configuration data, or a user-specified default if it is not found.
extern C_API SLONG ENV_get_value_number(const CBYTE *property, SLONG def, const CBYTE *section);

/// Updates the value of a string within the configuration data.
extern C_API void ENV_set_value_string(
    const CBYTE *property,
    const CBYTE *value,
    const CBYTE *section
);

/// Updates the value of an integer within the configuration data.
extern C_API void ENV_set_value_number(const CBYTE *property, SLONG value, const CBYTE *section);

/// Destroys a string allocated by `ENV_get_value_string`.
extern C_API void ENV_free_string(CBYTE *s);

#endif
