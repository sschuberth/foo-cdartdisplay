**Version 3.0, release build 145** (2011-Jul-28)
  * ADD: Add support for writing ratings to the playback statistics database via foo\_playcount.
  * CHG: Remove the requirement of SSE2 ([issue #5](https://code.google.com/p/foo-cdartdisplay/issues/detail?id=#5)).
  * FIX: Fix a potential crash when getting lyrics ([issue #5](https://code.google.com/p/foo-cdartdisplay/issues/detail?id=#5)).

**Version 3.0, release build 144** (2011-Apr-05)
  * ADD: Add a check for SSE2 instructions.
  * CHG: Compile using Visual Studio 2010.
  * CHG: Update to foobar2000 SDK 2011-03-11.
  * CHG: Miscellaneous code clean-ups and minor improvements.
  * FIX: Fix a bug caused by rounding issues when increasing the volume using the mouse wheel ([issue #1](https://code.google.com/p/foo-cdartdisplay/issues/detail?id=#1)).
  * FIX: Fix a NULL pointer exception when the path to CAD cannot be read from the registry ([issue #2](https://code.google.com/p/foo-cdartdisplay/issues/detail?id=#2)).

**Version 2.0.1, release build 100** (2010-Apr-23)
  * CHG: Update to foobar2000 SDK 2010-01-19 (minimum foobar2000 version requirement now is 1.0).
  * CHG: The component now requires a CPU with SSE2 instructions.
  * FIX: Fix the volume mapping between foobar2000's logarithmic and CAD's linear scale.
  * FIX: Fix several texts and URLs to contain correct / updated information.

**Version 2.0.1, release build 86** (2009-Jan-31)
  * CHG: Make the source code available under LGPL at http://foo-cdartdisplay.googlecode.com/, which has the side-effect of the build numbers to change.
  * CHG: Update to foobar2000 SDK 2008-11-29.

**Version 2.0, final build 174** (2008-Aug-26)
  * CHG: Update to foobar2000 SDK 2008-07-20.

**Version 2.0, beta build 148** (2008-Jul-12)
  * ADD: Add some assertions to check for string truncation.
  * CHG: Update to foobar2000 SDK 2008-07-10.
  * CHG: Rework some buffer copy operations for security and performance.
  * CHG: Optimize some release mode compiler settings.

**Version 2.0, beta build 134** (2008-Apr-24)
  * ADD: Add support for %bitrate% tag.
  * CHG: Update to foobar2000 SDK 2008-04-19 (minimum foobar2000 version requirement now is 0.9.5).

**Version 2.0, beta build 129** (2008-Apr-03)
  * CHG: More detailed error message.
  * FIX: Do not try to run CAD if it is already running, send a notification instead.

**Version 2.0, beta build 116** (2007-Nov-12)
  * ADD: First official release.

_Legend: ADD = Added, CHG = Changed, FIX = Fixed_
