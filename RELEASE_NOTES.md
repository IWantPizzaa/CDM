# Unreleased

- Integrated official commits `70c84e3f` and `75d0c535`.
- Runway Pressure now defaults to `-1` (globally disabled) when the XML setting
  is absent.
- For CTOT flights, startup advance now uses the average planned runway wait
  of nearby non-CTOT departures sharing the runway or a dependent runway/SID,
  capped by the configured pressure. Time comparisons handle midnight.
- Preserved the fork's API authentication, EuroScope Plugin Bridge, and CTOT
  delay exclusions. No release or version bump accompanies this source update.

# CDM 2.31.7b

Windows x86 (32-bit) beta for EuroScope, integrating the three official updates
through `rpuig2001/CDM` commit `a9d55b2b` after the previous 2.31.6b release.
This fork uses 2.31.7b to distinguish this build; upstream still reports 2.31.6b.

## Changes since 2.31.6b

- Renamed ERWT to Runway Pressure, including XML configuration, rate values,
  and runtime commands.
- Refreshed flight times even when flights are outside the CDM display window.
- Integrated the official runway sequencing changes from `a9d55b2b`.
- Preserved authenticated vIFF builds, the EuroScope Plugin Bridge and its
  timer, and the fork's CTOT delay behavior.

## Configuration migration

If you enabled ERWT in the previous beta, replace `<erwt minutes="15"/>` with
`<runwayPressure minutes="15"/>` and use `.cdm runwaypressure` instead of
`.cdm erwt`. Set `.cdm runwaypressure -1` to disable pressure globally.
A default of zero now permits positive per-runway values from `rate.txt` or
runtime overrides; omitted runway values use the default. The optional third
value in each `rate.txt` tuple retains its position.

Replace only the Release `CDM.dll` and keep your other local configuration.
The bundled XML is unchanged from 2.30.3 and contains no ERWT setting.

## Previous release: 2.31.6b

Windows x86 (32-bit) plugin sources for EuroScope, integrating all 19 official
commits since 2.30.3 through `rpuig2001/CDM` commit `b5a26bff` (12 November 2026).
The official plugin version is a beta.

## Changes since 2.30.3

- Added Estimated Runway Waiting Time (ERWT), including commit `01cf5ca9`:
  XML defaults, optional per-runway values in `rate.txt`, and `.cdm erwt`
  commands. ERWT allows earlier startup while retaining the sequenced TTOT.
- Added the `Auto Start-up` tag item and `Toggle Auto Start-up` function.
  Its armed/assigned state is shared through flight strips between controllers.
- Added master-airport synchronization with vIFF for direct VATSIM connections.
- Integrated CTOT sequencing fixes after TOBT changes, optional immediate TOBT
  recalculation, and TTOT conflict checks against frozen flights and BMI slots.
- Integrated de-icing data updates, ATFCM informed status for relevant flights,
  support for DEP controller positions, and safer background-task shutdown.
- Preserved build-time API authentication, the `com.viffsys.cdm` bridge provider,
  its periodic publication, and the fork's exclusion of CTOT flights from
  Startup Delay and Departure Delay commands.

## Configuration

Existing XML and `rate.txt` files remain compatible. ERWT is disabled when the
XML setting is absent or zero. To enable it, add `<erwt minutes="15"/>` inside
the existing `<CDM>` element, or run `.cdm erwt 15`. Per-runway examples:
`.cdm erwt LFPG/26R 10` and `.cdm erwt LFPG/26R default`.

The optional `<recalculateOnTobtUpdate mode="true"/>` setting enables immediate
background-sequence recalculation after TOBT updates; it is disabled by default.
To use automatic startup, add `Auto Start-up` to the EuroScope departure list
and bind its click action to `Toggle Auto Start-up`.

Replace only the compiled Release `CDM.dll` when installing. Keep existing
airport, rate, taxi-zone, color, and XML files. Authenticated builds still
require `CDM_API_KEY` through the environment or CMake configuration.

## Previous release: 2.30.3

Windows x86 (32-bit) build for EuroScope, based on the official 2.30.3 patch
at `rpuig2001/CDM` commit `0ddb543b`.

## Changes since 2.30.2

- Updated the vIFF authentication credential used by the Release DLL. Builds
  still receive it through `CDM_API_KEY`; it is not stored in the source tree.
- Preserved the fork's EuroScope Plugin Bridge provider and CTOT delay behavior.
- No XML change is required; keep the existing `CDMconfig.xml` and other local
  configuration files.

## Previous release: 2.30.2

Windows x86 (32-bit) build for EuroScope, integrating the official 2.30.1 and
2.30.2 patches through `rpuig2001/CDM` commit `9e7c4d0d`.

## Changes since 2.30

- Exclude flights with a CTOT from Startup Delay and Departure Delay commands.
- Accept multiple runways in traffic-rate `noDep` and `noArr` settings.
- Use the current vIFF endpoint, `https://viffsys.up.railway.app`.
- Preserve the fork's authenticated API build and EuroScope Plugin Bridge provider.

## Previous release: 2.30

Windows x86 (32-bit) build for EuroScope, based on rpuig2001/CDM `master`
at `7570ecc0` (14 October 2026).

## Changes since 2.29.1

- Integrated the upstream 2.30 plugin, including its reorganized source tree,
  advanced-airport handling and ATFCM messaging updates.
- Kept the authenticated CDM API build using `CDM_API_KEY`. The upstream test
  credential no longer replaces the configured key when the plugin loads.
- Kept the EuroScope Plugin Bridge provider `com.viffsys.cdm` in
  `src/core/CDM_Bridge.cpp`. The ECFMP field remains in its schema for
  compatibility and is empty because upstream removed ECFMP data.
- Existing `CDMconfig.xml` files remain usable; new optional settings use
  defaults when absent.

## Install or update

Close EuroScope, back up the current plugin folder, and replace only `CDM.dll`
with the Release build. Keep the existing XML and other local configuration
files. The build must receive `CDM_API_KEY` through its environment or CMake
configuration; the key is embedded in the DLL.

## Previous release: 2.29.1

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
