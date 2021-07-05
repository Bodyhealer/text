// =============================================================================
//
// ztd.text
// Copyright © 2021 JeanHeyd "ThePhD" Meneide and Shepherd's Oasis, LLC
// Contact: opensource@soasis.org
//
// Commercial License Usage
// Licensees holding valid commercial ztd.text licenses may use this file in
// accordance with the commercial license agreement provided with the
// Software or, alternatively, in accordance with the terms contained in
// a written agreement between you and Shepherd's Oasis, LLC.
// For licensing terms and conditions see your agreement. For
// further information contact opensource@soasis.org.
//
// Apache License Version 2 Usage
// Alternatively, this file may be used under the terms of Apache License
// Version 2.0 (the "License") for non-commercial use; you may not use this
// file except in compliance with the License. You may obtain a copy of the
// License at
//
//		http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// ============================================================================>

#include <ztd/text/encode.hpp>
#include <ztd/text/encoding.hpp>

#include <catch2/catch.hpp>

#include <ztd/text/tests/basic_unicode_strings.hpp>

TEST_CASE("text/encode/core", "basic usages of encode function do not explode") {
	SECTION("execution") {
		ztd::text::execution encoding {};
		std::string result0 = ztd::text::encode(
		     ztd::text::tests::u32_basic_source_character_set, encoding, ztd::text::replacement_handler {});
		REQUIRE(result0 == ztd::text::tests::basic_source_character_set);

		if (ztd::text::contains_unicode_encoding(encoding)) {
			std::string result1 = ztd::text::encode(ztd::text::tests::u32_unicode_sequence_truth_native_endian,
			     encoding, ztd::text::replacement_handler {});
			REQUIRE(result1 == ztd::text::tests::unicode_sequence_truth_native_endian);
		}
	}
	SECTION("wide_execution") {
		ztd::text::wide_execution encoding {};
		std::wstring result0 = ztd::text::encode(
		     ztd::text::tests::u32_basic_source_character_set, encoding, ztd::text::replacement_handler {});
		REQUIRE(result0 == ztd::text::tests::w_basic_source_character_set);

		if (ztd::text::contains_unicode_encoding(encoding)) {
			std::wstring result1 = ztd::text::encode(ztd::text::tests::u32_unicode_sequence_truth_native_endian,
			     encoding, ztd::text::replacement_handler {});
			REQUIRE(result1 == ztd::text::tests::w_unicode_sequence_truth_native_endian);
		}
	}
	SECTION("literal") {
		ztd::text::literal encoding {};
		std::string result0 = ztd::text::encode(
		     ztd::text::tests::u32_basic_source_character_set, encoding, ztd::text::replacement_handler {});
		REQUIRE(result0 == ztd::text::tests::basic_source_character_set);

		if (ztd::text::contains_unicode_encoding(encoding)) {
			std::string result1 = ztd::text::encode(ztd::text::tests::u32_unicode_sequence_truth_native_endian,
			     encoding, ztd::text::replacement_handler {});
			REQUIRE(result1 == ztd::text::tests::unicode_sequence_truth_native_endian);
		}
	}
	SECTION("wide_literal") {
		ztd::text::wide_literal encoding {};
		std::wstring result0 = ztd::text::encode(ztd::text::tests::u32_basic_source_character_set, encoding);
		REQUIRE(result0 == ztd::text::tests::w_basic_source_character_set);

		if (ztd::text::contains_unicode_encoding(encoding)) {
			std::wstring result1 = ztd::text::encode(ztd::text::tests::u32_unicode_sequence_truth_native_endian,
			     encoding, ztd::text::replacement_handler {});
			REQUIRE(result1 == ztd::text::tests::w_unicode_sequence_truth_native_endian);
		}
	}
	SECTION("utf8") {
		ztd::text::utf8 encoding {};
		std::basic_string<ztd::uchar8_t> result0
		     = ztd::text::encode(ztd::text::tests::u32_basic_source_character_set, encoding);
		REQUIRE(result0 == ztd::text::tests::u8_basic_source_character_set);

		std::basic_string<ztd::uchar8_t> result1
		     = ztd::text::encode(ztd::text::tests::u32_unicode_sequence_truth_native_endian, encoding);
		REQUIRE(result1 == ztd::text::tests::u8_unicode_sequence_truth_native_endian);
	}
	SECTION("utf16") {
		ztd::text::utf16 encoding {};
		std::u16string result0 = ztd::text::encode(ztd::text::tests::u32_basic_source_character_set, encoding);
		REQUIRE(result0 == ztd::text::tests::u16_basic_source_character_set);

		std::u16string result1
		     = ztd::text::encode(ztd::text::tests::u32_unicode_sequence_truth_native_endian, encoding);
		REQUIRE(result1 == ztd::text::tests::u16_unicode_sequence_truth_native_endian);
	}
	SECTION("utf32") {
		ztd::text::utf32 encoding {};
		std::u32string result0 = ztd::text::encode(ztd::text::tests::u32_basic_source_character_set, encoding);
		REQUIRE(result0 == ztd::text::tests::u32_basic_source_character_set);

		std::u32string result1
		     = ztd::text::encode(ztd::text::tests::u32_unicode_sequence_truth_native_endian, encoding);
		REQUIRE(result1 == ztd::text::tests::u32_unicode_sequence_truth_native_endian);
	}
}
