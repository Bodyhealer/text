.. =============================================================================
..
.. ztd.text
.. Copyright © 2021 JeanHeyd "ThePhD" Meneide and Shepherd's Oasis, LLC
.. Contact: opensource@soasis.org
..
.. Commercial License Usage
.. Licensees holding valid commercial ztd.text licenses may use this file in
.. accordance with the commercial license agreement provided with the
.. Software or, alternatively, in accordance with the terms contained in
.. a written agreement between you and Shepherd's Oasis, LLC.
.. For licensing terms and conditions see your agreement. For
.. further information contact opensource@soasis.org.
..
.. Apache License Version 2 Usage
.. Alternatively, this file may be used under the terms of Apache License
.. Version 2.0 (the "License") for non-commercial use; you may not use this
.. file except in compliance with the License. You may obtain a copy of the
.. License at
..
..		http:..www.apache.org/licenses/LICENSE-2.0
..
.. Unless required by applicable law or agreed to in writing, software
.. distributed under the License is distributed on an "AS IS" BASIS,
.. WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
.. See the License for the specific language governing permissions and
.. limitations under the License.
..
.. =============================================================================

Configuring the Library
=======================

There are various configuration macros and CMake/build-time switches that will change the behavior of the library or attempt to use different 

.. _config-ZTD_TEXT_USE_CUNEICODE:

- ``ZTD_TEXT_USE_CUNEICODE`` (CMake: ``ZTD_TEXT_USE_CUNEICODE``)
	- Enables use of the `Cuneicode project <https://ztdcuneicode.rtfd.io>`_, which has low-level C routines for converting from one
	- Makes the ``ztd::text::cnc_encoding`` available (accessible directly VIA ``#include <ztd/text/cnc_encoding.hpp>``)
	- Default: off.
	- Turned on if the special ``__has_include`` directive is present with the compiler and ``__has_include(<ztd/cuneicode/cuneicode.h>)`` works.

.. _config-ZTD_TEXT_USE_ICONV:

- ``ZTD_TEXT_USE_ICONV`` (CMake: ``ZTD_TEXT_USE_ICONV``)
	- Enables use of the `iconv project <https://www.gnu.org/software/libiconv/>`_.
	- Attempts to load it from the system at runtime using ``GetProcAddress``, ``dlopen``/``dlsym``/``dlclose``.
	- Makes the ``ztd::text::iconv_encoding`` available (accessible directly VIA ``#include <ztd/text/iconv_encoding.hpp>``).
	- Default: off.
	- Not turned on normally under any conditions.

.. _config-ZTD_TEXT_COMPILE_TIME_ENCODING_NAME:

- ``ZTD_TEXT_COMPILE_TIME_ENCODING_NAME``
	- Gives the :doc:`ztd::text::literal encoding</api/encodings/literal>` a name that matches what the encoding of string literals (``"👍"``) are.
	- Overrides any of library's heuristics and shenanigans to determine the string literal encoding.
	- If this does not match the actual string literal encoding, it can cause ☢️☢️Undefined Behavior☢️☢️.
	- Default: A complex set of platform checks. See :doc:`ztd::text::literal encoding</api/encodings/literal>` for more details.
	- Not turned on normally under any circumstances.

.. _config-ZTD_TEXT_YES_PLEASE_FUCK_MY_LITERALS_UP_I_MEAN_IT:

- ``ZTD_TEXT_YES_PLEASE_FUCK_MY_LITERALS_UP_I_MEAN_IT``
	- Enables ignoring the fact that the string literal (``"🤷‍♀️"``) encoding cannot be determined/discovered on the given platform for the :doc:`ztd::text::literal encoding</api/encodings/literal>`.
	- Will cause ☢️☢️Undefined Behavior☢️☢️ if a string literal or wide string literal is encoded or decoded to/from and the encoding does not match whatever pot-shot guess the system takes.
	- Default: off
	- Not turned on normally under any conditions.
	- **Please don't use this unless you have some really, really weird setup that requires messing everything up...**

.. _config-ZTD_TEXT_COMPILE_TIME_WIDE_ENCODING_NAME:

- ``ZTD_TEXT_COMPILE_TIME_WIDE_ENCODING_NAME``
	- Gives the :doc:`ztd::text::wide_literal encoding</api/encodings/wide_literal>` a name that matches what the encoding of wide string literals (``L"👍"``) are.
	- Overrides any of library's heuristics and shenanigans to determine the wide string literal encoding.
	- If this does not match the actual wide string literal encoding, it can cause ☢️☢️Undefined Behavior☢️☢️.
	- Default: A complex set of platform checks. See :doc:`ztd::text::wide_literal encoding</api/encodings/wide_literal>` for more details.
	- Not turned on normally under any circumstances.

.. _config-ZTD_TEXT_YES_PLEASE_FUCK_MY_WIDE_LITERALS_UP_I_MEAN_IT:

- ``ZTD_TEXT_YES_PLEASE_FUCK_MY_WIDE_LITERALS_UP_I_MEAN_IT``
	- Enables ignoring the fact that the wide string literal (``L"🤷‍♀️"``) encoding cannot be determined/discovered on the given platform for the :doc:`ztd::text::wide_literal encoding</api/encodings/wide_literal>`.
	- Will cause ☢️☢️Undefined Behavior☢️☢️ if a string literal or wide string literal is encoded or decoded to/from and the encoding does not match whatever pot-shot guess the system takes.
	- Default: off
	- Not turned on normally under any conditions.
	- **Please don't use this unless you have some really, really weird setup that requires messing everything up...**
