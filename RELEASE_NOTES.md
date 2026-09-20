# CDM 2.29.1

Windows x86 (32-bit) build for EuroScope, from the IWantPizzaa/CDM fork.

## Changes

- Fixed CDM API authentication: the plugin now receives its API key at build time instead of sending an empty key. Includes PR #1 by @goulven04.
- Added environment-variable and GitHub Actions secret support for `CDM_API_KEY`, with required-key validation for authenticated builds.
- Kept the optional EuroScope Plugin Bridge integration (`com.viffsys.cdm`) for consumers such as vSMR.
- Corrected Release build/test configuration and restricted CI artifacts to the plugin DLL, excluding generated secrets, caches, objects, and debug symbols.
- Added eight API-key configuration checks and aligned the plugin and project version to 2.29.1.

## Install or update

1. Close EuroScope and back up your existing CDM plugin folder.
2. Download `CDM-2.29.1.zip` and replace only `CDM.dll` in that folder, or use the standalone `CDM.dll` asset.
3. Keep your existing `CDMconfig.xml`, airport data, rates, taxi zones, colors, and other local configuration files. They are not included or overwritten by this release.
4. Start EuroScope and load the DLL through **Other Settings > Plug-ins** if it is not already configured.

This is a plugin update, not a complete airport/sector configuration package.
The distributed DLL is already built with API authentication; end users do not
need to set a build environment variable. The API key is embedded in the DLL,
as required by CDM's client authentication design.

For optional vSMR data integration, load EuroScope Plugin Bridge separately.
The bridge DLL is not bundled. `.esb providers` should list `com.viffsys.cdm`.

`SHA256SUMS.txt` contains checksums for the DLL and ZIP. Source code is available
at the matching `2.29.1` tag. The ZIP includes the project license and third-party
dependency license notices.
