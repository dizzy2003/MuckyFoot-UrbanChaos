// Copyright (c) 2017 FaultyRAM
//
// Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
// http://www.apache.org/licenses/LICENSE-2.0> or the MIT license <LICENSE-MIT
// or http://opensource.org/licenses/MIT>, at your option. This file may not be
// copied, modified, or distributed except according to those terms.

//! Configuration file support for OpenChaos.

#![cfg_attr(feature = "clippy", forbid(clippy))]
#![cfg_attr(feature = "clippy", forbid(clippy_internal))]
#![cfg_attr(feature = "clippy", forbid(clippy_pedantic))]
#![cfg_attr(feature = "clippy", forbid(clippy_restrictions))]
#![forbid(warnings)]
#![forbid(box_pointers)]
#![forbid(fat_ptr_transmutes)]
#![forbid(missing_copy_implementations)]
#![forbid(missing_debug_implementations)]
#![forbid(missing_docs)]
#![forbid(trivial_casts)]
#![forbid(trivial_numeric_casts)]
#![forbid(unsafe_code)]
#![forbid(unused_extern_crates)]
#![forbid(unused_import_braces)]
#![deny(unused_qualifications)]
#![forbid(unused_results)]
#![forbid(variant_size_differences)]

#[macro_use]
extern crate serde_derive;
extern crate toml;

use std::fs::OpenOptions;
use std::io;
use std::io::{Read, Write};
use std::path::Path;

#[derive(Clone, Debug, Deserialize, Serialize)]
#[serde(default)]
/// A set of configuration data.
pub struct Config {
    #[serde(rename = "Game")]
    /// Game configuration data.
    pub game: GameConfig,
    #[serde(rename = "LocalInstall")]
    /// Local installation configuration data.
    pub local_install: LocalInstallConfig,
    #[serde(rename = "TextureClumps")]
    /// Texture clump configuration data.
    pub texture_clumps: TextureClumpConfig,
    #[serde(rename = "Render")]
    /// Renderer configuration data.
    pub render: RendererConfig,
    #[serde(rename = "Audio")]
    /// Audio configuration data.
    pub audio: AudioConfig,
    #[serde(rename = "Joypad")]
    /// Joypad configuration data.
    pub joypad: JoypadConfig,
    #[serde(rename = "Keyboard")]
    /// Keyboard configuration data.
    pub keyboard: KeyboardConfig,
    #[serde(rename = "Gamma")]
    /// Gamma configuration data.
    pub gamma: GammaConfig,
    #[serde(rename = "Movie")]
    /// Movie configuration data.
    pub movie: MovieConfig,
}

#[derive(Clone, Debug, Deserialize, Serialize)]
#[serde(default)]
/// Game configuration.
pub struct GameConfig {
    /// A text file to load localized strings from.
    pub language: String,
    /// Whether the scanner follows the camera or the player character.
    pub scanner_follows: ScannerFollows,
    /// If `true`, the engine will behave as though it is running on a PlayStation.
    pub iamapsx: bool,
}

#[derive(Clone, Copy, Debug, Deserialize, Serialize)]
#[serde(default)]
/// Local installation configuration.
pub struct LocalInstallConfig {
    /// If `true`, look for textures under the game's installation directory.
    ///
    /// If `false`, look for textures on the CD-ROM.
    pub textures: bool,
    /// If `true`, look for sound effects under the game's installation directory.
    ///
    /// If `false`, look for sound effects on the CD-ROM.
    pub sfx: bool,
    /// If `true`, look for speech data under the game's installation directory.
    ///
    /// If `false`, look for speech data on the CD-ROM.
    pub speech: bool,
    /// If `true`, look for movies under the game's installation directory.
    ///
    /// If `false`, look for movies on the CD-ROM.
    pub movies: bool,
}

#[derive(Clone, Copy, Debug, Deserialize, Serialize)]
#[serde(default)]
/// Texture clump configuration.
pub struct TextureClumpConfig {
    /// If `true`, use texture clumps.
    pub enable_clumps: bool,
}

#[derive(Clone, Copy, Debug, Deserialize, Serialize)]
#[serde(default)]
/// Renderer configuration.
pub struct RendererConfig {
    /// If `true`, try to guess optimal renderer configuration.
    pub estimate_detail_levels: bool,
    /// If `true`, enable stars.
    pub detail_stars: bool,
    /// If `true`, enable shadows.
    pub detail_shadows: bool,
    /// If `true`, enable moon reflections.
    pub detail_moon_reflection: bool,
    /// If `true`, enable people reflections.
    pub detail_people_reflection: bool,
    /// If `true`, enable puddles.
    pub detail_puddles: bool,
    /// If `true`, enable dirt.
    pub detail_dirt: bool,
    /// If `true`, enable mist.
    pub detail_mist: bool,
    /// If `true`, enable rain.
    pub detail_rain: bool,
    /// If `true`, enable the skyline.
    pub detail_skyline: bool,
    /// If `true`, enable texture filtering.
    pub detail_filter: bool,
    /// If `true`, enable perspective correction.
    pub detail_perspective: bool,
    /// If `true`, enable crinkles.
    pub detail_crinkles: bool,
    /// If `true`, uses a DirectX 5-specific fix.
    pub fix_directx: bool,
    /// An index specifying which video card to use.
    pub video_card: VideoCard,
    /// If `true`, render in 24-bit colour.
    ///
    /// If `false`, render in 16-bit colour.
    pub video_truecolour: bool,
    /// Which display resolution to use.
    pub video_res: DisplayResolution,
    #[serde(rename = "Adami_lighting")]
    /// If `true`, use Adami lighting.
    pub adami_lighting: bool,
    /// The maximum number of frames per second to render.
    pub max_frame_rate: usize,
    /// The maximum draw distance.
    ///
    /// Note that values above `22` tend to causes crashes.
    pub draw_distance: usize,
}

#[derive(Clone, Debug, Deserialize, Serialize)]
#[serde(default)]
/// Audio configuration.
pub struct AudioConfig {
    /// Apparently renders a dialog box if `true`, but changing it didn't seem to do anything...
    pub run_sound_dialog: bool,
    /// The ambient volume level.
    ///
    /// Ranges from `0`-`127`.
    pub ambient_volume: u8,
    /// The background music volume level.
    ///
    /// Ranges from `0`-`127`.
    pub music_volume: u8,
    /// The sound effect volume level.
    ///
    /// Ranges from `0`-`127`.
    pub fx_volume: u8,
    #[serde(rename = "3D_sound_driver")]
    /// The 3D audio backend to use.
    pub sound_driver: String,
    /// If `true`, the audio engine will mimic the behaviour of the PlayStation version's audio
    /// engine.
    pub dodgy_psx_sound: bool,
}

#[derive(Copy, Clone, Debug, Deserialize, Serialize)]
/// Joypad configuration.
pub struct JoypadConfig {
    /// An index specifying which button to use for kicking.
    pub joypad_kick: u8,
    /// An index specifying which button to use for punching.
    pub joypad_punch: u8,
    /// An index specifying which button to use for jumping.
    pub joypad_jump: u8,
    /// An index specifying which button to use for context-sensitive commands.
    pub joypad_action: u8,
    /// An index specifying which button to use for sprinting.
    pub joypad_move: u8,
    /// An index specifying which button to use as the "start" button.
    pub joypad_start: u8,
    /// An index specifying which button to use as the "select" button.
    pub joypad_select: u8,
    /// An index specifying which button to use for re-centering the camera.
    pub joypad_camera: u8,
    /// An index specifying which button to use for rotating the camera to the left.
    pub joypad_cam_left: u8,
    /// An index specifying which button to use for rotating the camera to the right.
    pub joypad_cam_right: u8,
    /// An index specifying which button to use for first-person mode.
    pub joypad_1stperson: u8,
}

#[derive(Clone, Copy, Debug, Deserialize, Serialize)]
#[serde(default)]
/// Keyboard configuration.
pub struct KeyboardConfig {
    /// An index specifying which key to use for moving left.
    pub keyboard_left: u16,
    /// An index specifying which key to use for moving right.
    pub keyboard_right: u16,
    /// An index specifying which key to use for moving forwards.
    pub keyboard_forward: u16,
    /// An index specifying which key to use for moving backwards.
    pub keyboard_back: u16,
    /// An index specifying which key to use for punching.
    pub keyboard_punch: u16,
    /// An index specifying which key to use for kicking.
    pub keyboard_kick: u16,
    /// An index specifying which key to use for context-sensitive actions.
    pub keyboard_action: u16,
    /// An index specifying which key to use for sprinting.
    pub keyboard_run: u16,
    /// An index specifying which key to use for jumping.
    pub keyboard_jump: u16,
    /// An index specifying which key to use as the "start" button.
    pub keyboard_start: u16,
    /// An index specifying which key to use as the "select" button.
    pub keyboard_select: u16,
    /// An index specifying which key to use for re-centering the camera.
    pub keyboard_camera: u16,
    /// An index specifying which key to use for rotating the camera towards the left.
    pub keyboard_cam_left: u16,
    /// An index specifying which key to use for rotating the camera towards the right.
    pub keyboard_cam_right: u16,
    /// An index specifying which key to use for first-person mode.
    pub keyboard_1stperson: u16,
}

#[derive(Clone, Copy, Debug, Deserialize, Serialize)]
#[serde(default)]
/// Gamma configuration.
pub struct GammaConfig {
    #[serde(rename = "BlackPoint")]
    /// The gamma black point.
    pub black_point: u16,
    #[serde(rename = "WhitePoint")]
    /// The gamma white point.
    pub white_point: u16,
}

#[derive(Clone, Copy, Debug, Deserialize, Serialize)]
#[serde(default)]
/// Movie configuration.
pub struct MovieConfig {
    /// If `true`, play movies.
    pub play_movie: bool,
}

#[derive(Clone, Copy, Debug, Deserialize, Serialize)]
/// Scanner follow targets.
pub enum ScannerFollows {
    /// The scanner follows the camera.
    Camera = 0,
    /// The scanner follows the player character.
    Character = 1,
}

#[derive(Clone, Copy, Debug, Deserialize, Serialize)]
/// Video card selections.
pub enum VideoCard {
    /// The primary video card.
    Primary = 0,
    /// The secondary video card.
    Secondary = 1,
    /// Software rendering.
    Software = 2,
}

#[derive(Clone, Copy, Debug, Deserialize, Serialize)]
/// Display resolutions.
pub enum DisplayResolution {
    /// 320x240.
    Res320x240 = 0,
    /// 512x384.
    Res512x384 = 1,
    /// 640x480.
    Res640x480 = 2,
    /// 800x600.
    Res800x600 = 3,
    /// 1024x768.
    Res1024x768 = 4,
}

/// Loads configuration data from a file.
pub fn from_file<P: AsRef<Path>>(path: P) -> io::Result<Config> {
    OpenOptions::new()
        .read(true)
        .open(path)
        .and_then(|mut f| {
            let mut s = String::new();
            f.read_to_string(&mut s).map(|_| s)
        })
        .and_then(|s| {
            toml::from_str(&s).map_err(|e| io::Error::new(io::ErrorKind::InvalidData, e))
        })
}

/// Writes configuration data to a file.
pub fn to_file<P: AsRef<Path>>(config: &Config, path: P) -> io::Result<()> {
    toml::to_string(config)
        .map_err(|e| io::Error::new(io::ErrorKind::InvalidData, e))
        .and_then(|s| {
            OpenOptions::new()
                .write(true)
                .truncate(true)
                .open(path)
                .map(|f| (f, s))
        })
        .and_then(|(mut f, s)| f.write_all(s.as_bytes()))
}

impl Config {
    /// Creates a new set of configuration data with default values.
    pub fn new() -> Config {
        Config {
            game: GameConfig::default(),
            local_install: LocalInstallConfig::default(),
            texture_clumps: TextureClumpConfig::default(),
            render: RendererConfig::default(),
            audio: AudioConfig::default(),
            joypad: JoypadConfig::default(),
            keyboard: KeyboardConfig::default(),
            gamma: GammaConfig::default(),
            movie: MovieConfig::default(),
        }
    }
}

impl Default for Config {
    fn default() -> Config {
        Config::new()
    }
}

impl GameConfig {
    /// Creates a new set of game configuration data with default values.
    pub fn new() -> GameConfig {
        GameConfig {
            language: "text/lang_english.txt".to_owned(),
            scanner_follows: ScannerFollows::Character,
            iamapsx: false,
        }
    }
}

impl Default for GameConfig {
    fn default() -> GameConfig {
        GameConfig::new()
    }
}

impl LocalInstallConfig {
    /// Creates a new set of local installation configuration data with default values.
    pub fn new() -> LocalInstallConfig {
        // Since CD-ROM support is on the chopping block, just assume everything is installed
        // locally.
        LocalInstallConfig {
            textures: true,
            sfx: true,
            speech: true,
            movies: true,
        }
    }
}

impl Default for LocalInstallConfig {
    fn default() -> LocalInstallConfig {
        LocalInstallConfig::new()
    }
}

impl TextureClumpConfig {
    /// Creates a new set of texture clump configuration data with default values.
    pub fn new() -> TextureClumpConfig {
        TextureClumpConfig { enable_clumps: true }
    }
}

impl Default for TextureClumpConfig {
    fn default() -> TextureClumpConfig {
        TextureClumpConfig::new()
    }
}

impl RendererConfig {
    /// Creates a new set of renderer configuration data with default values.
    pub fn new() -> RendererConfig {
        RendererConfig {
            estimate_detail_levels: false,
            detail_stars: true,
            detail_shadows: true,
            detail_moon_reflection: true,
            detail_people_reflection: true,
            detail_puddles: true,
            detail_dirt: true,
            detail_mist: true,
            detail_rain: true,
            detail_skyline: true,
            detail_filter: true,
            detail_perspective: true,
            detail_crinkles: true,
            fix_directx: false,
            video_card: VideoCard::default(),
            video_truecolour: true,
            video_res: DisplayResolution::default(),
            adami_lighting: true,
            max_frame_rate: 60,
            draw_distance: 22,
        }
    }
}

impl Default for RendererConfig {
    fn default() -> RendererConfig {
        RendererConfig::new()
    }
}

impl AudioConfig {
    /// Creates a new set of audio configuration data with default values.
    pub fn new() -> AudioConfig {
        AudioConfig {
            run_sound_dialog: true,
            ambient_volume: 127,
            music_volume: 127,
            fx_volume: 127,
            sound_driver: "Microsoft DirectSound3D software emulation".to_owned(),
            dodgy_psx_sound: false,
        }
    }
}

impl Default for AudioConfig {
    fn default() -> AudioConfig {
        AudioConfig::new()
    }
}

impl JoypadConfig {
    /// Creates a new set of joypad configuration data with default values.
    pub fn new() -> JoypadConfig {
        // Just going off the vanilla defaults. We need to redo the input engine anyway.
        JoypadConfig {
            joypad_kick: 4,
            joypad_punch: 3,
            joypad_jump: 0,
            joypad_action: 1,
            joypad_move: 7,
            joypad_start: 8,
            joypad_select: 2,
            joypad_camera: 6,
            joypad_cam_left: 9,
            joypad_cam_right: 10,
            joypad_1stperson: 5,
        }
    }
}

impl Default for JoypadConfig {
    fn default() -> JoypadConfig {
        JoypadConfig::new()
    }
}

impl KeyboardConfig {
    /// Creates a new set of keyboard configuration data with default values.
    pub fn new() -> KeyboardConfig {
        // Again, just going off the vanilla defaults.
        KeyboardConfig {
            keyboard_left: 203,
            keyboard_right: 205,
            keyboard_forward: 200,
            keyboard_back: 208,
            keyboard_punch: 44,
            keyboard_kick: 45,
            keyboard_action: 46,
            keyboard_run: 47,
            keyboard_jump: 48,
            keyboard_start: 15,
            keyboard_select: 28,
            keyboard_camera: 207,
            keyboard_cam_left: 211,
            keyboard_cam_right: 209,
            keyboard_1stperson: 30,
        }
    }
}

impl Default for KeyboardConfig {
    fn default() -> KeyboardConfig {
        KeyboardConfig::new()
    }
}

impl GammaConfig {
    /// Creates a new set of gamma configuration data with default values.
    pub fn new() -> GammaConfig {
        GammaConfig {
            black_point: 0,
            white_point: 256,
        }
    }
}

impl Default for GammaConfig {
    fn default() -> GammaConfig {
        GammaConfig::new()
    }
}

impl MovieConfig {
    /// Creates a new set of movie configuration data with default values.
    pub fn new() -> MovieConfig {
        MovieConfig { play_movie: true }
    }
}

impl Default for MovieConfig {
    fn default() -> MovieConfig {
        MovieConfig::new()
    }
}

impl Default for ScannerFollows {
    fn default() -> ScannerFollows {
        ScannerFollows::Character
    }
}

impl Default for VideoCard {
    fn default() -> VideoCard {
        VideoCard::Primary
    }
}

impl Default for DisplayResolution {
    fn default() -> DisplayResolution {
        DisplayResolution::Res640x480
    }
}
