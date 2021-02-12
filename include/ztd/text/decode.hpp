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
// =============================================================================

#pragma once

#ifndef ZTD_TEXT_DECODE_HPP
#define ZTD_TEXT_DECODE_HPP

#include <ztd/text/code_point.hpp>
#include <ztd/text/default_encoding.hpp>
#include <ztd/text/decode_result.hpp>
#include <ztd/text/error_handler.hpp>
#include <ztd/text/state.hpp>
#include <ztd/text/unbounded.hpp>
#include <ztd/text/is_unicode_code_point.hpp>

#include <ztd/text/detail/is_lossless.hpp>
#include <ztd/text/detail/encoding_range.hpp>
#include <ztd/text/detail/type_traits.hpp>
#include <ztd/text/detail/span.hpp>

#include <string>
#include <vector>
#include <string_view>

namespace ztd { namespace text {
	ZTD_TEXT_INLINE_ABI_NAMESPACE_OPEN_I_

	//////
	/// @addtogroup ztd_text_decode ztd::text::decode[_into]
	/// @brief These functions convert from a view of input into a view of output using either the inferred or
	/// specified encodings. If no error handler is provided, the equivalent of the @ref ztd::text::default_handler is
	/// used by default. If no associated state is provided for either the "to" or "from" encodings, one will be
	/// created with automatic storage duration (as a "stack" variable) for the provided encoding.
	/// @{
	//////

	//////
	/// @brief Converts the code units of the given @p __input view through the encoding to code points into the  @p
	/// __output view.
	///
	/// @param[in]     __input An input_view to read code units from and use in the decode operation that will
	/// produce code points.
	/// @param[in]     __encoding The encoding that will be used to decode the input's code points into
	/// output code units.
	/// @param[in]     __output An output_view to write code points to as the result of the decode operation from the
	/// intermediate code units.
	/// @param[in]     __error_handler The error handlers for the from and to encodings,
	/// respectively.
	/// @param[in,out] __state A reference to the associated state for the @p __encoding 's decode step.
	///
	/// @result A ztd::text::decode_result object that contains references to @p __state.
	///
	/// @remark This function detects whether or not the ADL extension point @c text_decode can be called with the
	/// provided parameters. If so, it will use that ADL extension point over the default implementation. Otherwise, it
	/// will loop over the two encodings and attempt to decode by repeatedly calling the encoding's required @c
	/// decode_one function.
	//////
	template <typename _Input, typename _Encoding, typename _Output, typename _ErrorHandler, typename _State>
	constexpr auto decode_into(_Input&& __input, _Encoding&& __encoding, _Output&& __output,
		_ErrorHandler&& __error_handler, _State& __state) {
		if constexpr (__detail::__is_detected_v<__detail::__detect_adl_text_decode, _Input, _Encoding, _Output,
			              _ErrorHandler, _State>) {
			return text_decode(::std::forward<_Input>(__input), ::std::forward<_Encoding>(__encoding),
				::std::forward<_Output>(__output), ::std::forward<_ErrorHandler>(__error_handler), __state);
		}
		else if constexpr (__detail::__is_detected_v<__detail::__detect_adl_internal_text_decode, _Input, _Encoding,
			                   _Output, _ErrorHandler, _State>) {
			return __text_decode(::std::forward<_Input>(__input), ::std::forward<_Encoding>(__encoding),
				::std::forward<_Output>(__output), ::std::forward<_ErrorHandler>(__error_handler), __state);
		}
		else {
			using _UInput             = __detail::__remove_cvref_t<_Input>;
			using _UOutput            = __detail::__remove_cvref_t<_Output>;
			using _InputValueType     = __detail::__range_value_type_t<_UInput>;
			using _IntermediateInput  = __detail::__reconstruct_t<::std::conditional_t<::std::is_array_v<_UInput>,
                    ::std::conditional_t<__detail::__is_character_v<_InputValueType>,
                         ::std::basic_string_view<_InputValueType>, ::std::span<const _InputValueType>>,
                    _UInput>>;
			using _IntermediateOutput = __detail::__reconstruct_t<_UOutput>;
			using _Result             = decltype(__encoding.decode_one(::std::declval<_IntermediateInput>(),
                    ::std::declval<_IntermediateOutput>(), __error_handler, __state));
			using _WorkingInput       = __detail::__remove_cvref_t<decltype(std::declval<_Result>().input)>;
			using _WorkingOutput      = __detail::__remove_cvref_t<decltype(std::declval<_Result>().output)>;
			using _UEncoding          = __detail::__remove_cvref_t<_Encoding>;
			using _UErrorHandler      = __detail::__remove_cvref_t<_ErrorHandler>;

			static_assert(__detail::__is_decode_lossless_or_deliberate_v<_UEncoding, _UErrorHandler>,
				"This decode is a lossy, non-injective operation. This means you may lose data that you did "
				"not "
				"intend "
				"to lose; specify a 'handler' error handler parameter to decode(in, encoding, handler, ...) "
				"or "
				"decode_into(in, encoding, out, handler, ...) explicitly in order to bypass this.");

			_WorkingInput __working_input(
				__detail::__reconstruct(::std::in_place_type<_WorkingInput>, ::std::forward<_Input>(__input)));
			_WorkingOutput __working_output(
				__detail::__reconstruct(::std::in_place_type<_WorkingOutput>, ::std::forward<_Output>(__output)));
			bool __handled_error = false;

			for (;;) {
				auto __result = __encoding.decode_one(__working_input, __working_output, __error_handler, __state);
				if (__result.error_code != encoding_error::ok) {
					return __result;
				}
				__handled_error |= __result.handled_error;
				__working_input  = ::std::move(__result.input);
				__working_output = ::std::move(__result.output);
				if (__detail::__adl::__adl_empty(__result.input)) {
					break;
				}
			}
			return _Result(
				::std::move(__working_input), ::std::move(__working_output), __state, encoding_error::ok, false);
		}
	}

	//////
	/// @brief Converts the code units of the given @p __input view through the encoding to code points into the  @p
	/// __output view.
	///
	/// @param[in]     __input An input_view to read code units from and use in the decode operation that will
	/// produce code points.
	/// @param[in]     __encoding The encoding that will be used to decode the input's code points into
	/// output code units.
	/// @param[in]     __output An output_view to write code points to as the result of the decode operation from the
	/// intermediate code units.
	/// @param[in]     __error_handler The error handlers for the from and to encodings,
	/// respectively.
	///
	/// @result A ztd::text::stateless_decode_result object that contains references to @p __state.
	///
	/// @remark Creates a default @c state using ztd::text::make_decode_state.
	//////
	template <typename _Input, typename _Encoding, typename _Output, typename _ErrorHandler>
	constexpr auto decode_into(
		_Input&& __input, _Encoding&& __encoding, _Output&& __output, _ErrorHandler&& __error_handler) {
		using _UEncoding = __detail::__remove_cvref_t<_Encoding>;
		using _State     = encoding_decode_state_t<_UEncoding>;

		_State __state         = make_decode_state(__encoding);
		auto __stateful_result = decode_into(::std::forward<_Input>(__input), ::std::forward<_Encoding>(__encoding),
			::std::forward<_Output>(__output), ::std::forward<_ErrorHandler>(__error_handler), __state);
		return __detail::__slice_to_stateless(::std::move(__stateful_result));
	}

	//////
	/// @brief Converts the code units of the given @p __input view through the encoding to code points into the  @p
	/// __output view.
	///
	/// @param[in]     __input An input_view to read code units from and use in the decode operation that will
	/// produce code points.
	/// @param[in]     __encoding The encoding that will be used to decode the input's code points into
	/// output code units.
	/// @param[in]     __output An output_view to write code points to as the result of the decode operation from the
	/// intermediate code units.
	///
	/// @result A ztd::text::stateless_decode_result object that contains references to @p __state.
	///
	/// @remark Creates a default @c error_handler that is similar to ztd::text::default_handler, but marked as
	/// careless.
	//////
	template <typename _Input, typename _Encoding, typename _Output>
	constexpr auto decode_into(_Input&& __input, _Encoding&& __encoding, _Output&& __output) {
		__detail::__careless_handler __handler {};
		return decode_into(::std::forward<_Input>(__input), ::std::forward<_Encoding>(__encoding),
			::std::forward<_Output>(__output), __handler);
	}

	//////
	/// @brief Converts the code units of the given @p __input view through the encoding to code points into the  @p
	/// __output view.
	///
	/// @param[in]     __input An input_view to read code units from and use in the decode operation that will
	/// produce code points.
	/// @param[in]     __output An output_view to write code points to as the result of the decode operation from the
	/// intermediate code units.
	///
	/// @result A ztd::text::stateless_decode_result object that contains references to @p __state.
	///
	/// @remark Creates a default @c encoding by figuring out the @c value_type of the @p __input, then passing that
	/// type into ztd::text::default_code_point_encoding_t. That encoding is that used to decode the input code units,
	/// by default.
	//////
	template <typename _Input, typename _Output>
	constexpr auto decode_into(_Input&& __input, _Output&& __output) {
		using _UInput   = __detail::__remove_cvref_t<_Input>;
		using _Encoding = default_code_unit_encoding_t<__detail::__range_value_type_t<_UInput>>;
		return decode_into(::std::forward<_Input>(__input), _Encoding {}, ::std::forward<_Output>(__output));
	}

	//////
	/// @brief Converts the code units of the given @p __input view through the encoding to code points the specified
	/// @p _OutputContainer type.
	///
	/// @tparam _OutputContainer The container type to serialize data into.
	///
	/// @param[in]     __input An input_view to read code units from and use in the decode operation that will
	/// produce code points.
	/// @param[in]     __encoding The encoding that will be used to decode the input's code points into
	/// output code units.
	/// @param[in]     __error_handler The error handlers for the from and to encodings,
	/// respectively.
	/// @param[in,out] __state A reference to the associated state for the @p __encoding 's decode step.
	///
	/// @result A ztd::text::decode_result object that contains references to @p __state and an output of type @p
	/// _OutputContainer.
	///
	/// @remark This function detects creates a container of type @p _OutputContainer and uses a typical @c
	/// std::back_inserter or @c std::push_back_inserter to fill in elements as it is written to. The result is then
	/// returned, with the @c .output value put into the container.
	//////
	template <typename _OutputContainer, typename _Input, typename _Encoding, typename _ErrorHandler, typename _State>
	constexpr auto decode_to(
		_Input&& __input, _Encoding&& __encoding, _ErrorHandler&& __error_handler, _State& __state) {
		using _UEncoding = __detail::__remove_cvref_t<_Encoding>;

		_OutputContainer __output {};
		if constexpr (__detail::__is_detected_v<__detail::__detect_adl_size, _Input>) {
			auto __output_size_hint = __detail::__adl::__adl_size(__input);
			__output_size_hint *= max_code_points_v<_UEncoding>;
			__output.reserve(__output_size_hint);
		}
		auto __stateful_result = decode_into(::std::forward<_Input>(__input), ::std::forward<_Encoding>(__encoding),
			unbounded_view(::std::back_inserter(__output)), ::std::forward<_ErrorHandler>(__error_handler), __state);
		return __detail::__replace_result_output(::std::move(__stateful_result), ::std::move(__output));
	}

	//////
	/// @brief Converts the code units of the given @p __input view through the encoding to code points the specified
	/// @p _OutputContainer type.
	///
	/// @tparam _OutputContainer The container type to serialize data into.
	///
	/// @param[in]     __input An input_view to read code units from and use in the decode operation that will
	/// produce code points.
	/// @param[in]     __encoding The encoding that will be used to decode the input's code points into
	/// output code units.
	/// @param[in]     __error_handler The error handlers for the from and to encodings,
	/// respectively.
	///
	/// @result A ztd::text::stateless_decode_result object whose output is of type @p _OutputContainer.
	///
	/// @remark This function creates a @c state using ztd::text::make_decode_state.
	//////
	template <typename _OutputContainer, typename _Input, typename _Encoding, typename _ErrorHandler>
	constexpr auto decode_to(_Input&& __input, _Encoding&& __encoding, _ErrorHandler&& __error_handler) {
		using _UEncoding = __detail::__remove_cvref_t<_Encoding>;
		using _State     = encoding_decode_state_t<_UEncoding>;
		_State __state   = make_decode_state(__encoding);
		return decode_to<_OutputContainer>(::std::forward<_Input>(__input), ::std::forward<_Encoding>(__encoding),
			::std::forward<_ErrorHandler>(__error_handler), __state);
	}

	//////
	/// @brief Converts the code units of the given @p __input view through the encoding to code points the specified
	/// @p _OutputContainer type.
	///
	/// @tparam _OutputContainer The container type to serialize data into.
	///
	/// @param[in]     __input An input_view to read code units from and use in the decode operation that will
	/// produce code points.
	/// @param[in]     __encoding The encoding that will be used to decode the input's code points into
	/// output code units.
	///
	/// @result A ztd::text::stateless_decode_result object whose output is of type @p _OutputContainer.
	///
	/// @remark This function creates a @c handler using ztd::text::default_handler, but marks it as careless.
	//////
	template <typename _OutputContainer, typename _Input, typename _Encoding>
	constexpr auto decode_to(_Input&& __input, _Encoding&& __encoding) {
		__detail::__careless_handler __handler {};
		return decode_to<_OutputContainer>(
			::std::forward<_Input>(__input), ::std::forward<_Encoding>(__encoding), __handler);
	}

	//////
	/// @brief Converts the code units of the given @p __input view through the encoding to code points the specified
	/// @p _OutputContainer type.
	///
	/// @tparam _OutputContainer The container type to serialize data into.
	///
	/// @param[in]     __input An input_view to read code units from and use in the decode operation that will
	/// produce code points.
	///
	/// @result A ztd::text::stateless_decode_result object whose output is of type @p _OutputContainer.
	///
	/// @remark This function creates an @c encoding by using the @c value_type of the @p __input which is then passed
	/// through the ztd::text::default_code_point_encoding type to get the default desired encoding.
	//////
	template <typename _OutputContainer, typename _Input>
	constexpr auto decode_to(_Input&& __input) {
		using _UInput   = __detail::__remove_cvref_t<_Input>;
		using _Encoding = default_code_unit_encoding_t<__detail::__range_value_type_t<_UInput>>;
		return decode_to<_OutputContainer>(::std::forward<_Input>(__input), _Encoding {});
	}

	//////
	/// @brief Converts the code units of the given @p __input view through the encoding to code points the specified
	/// @p _OutputContainer type.
	///
	/// @tparam _OutputContainer The container type to serialize data into.
	///
	/// @param[in]     __input An input_view to read code units from and use in the decode operation that will
	/// produce code points.
	/// @param[in]     __encoding The encoding that will be used to decode the input's code points into
	/// output code units.
	/// @param[in]     __error_handler The error handlers for the from and to encodings,
	/// respectively.
	/// @param[in,out] __state A reference to the associated state for the @p __encoding 's decode step.
	///
	/// @result An object of type @p _OutputContainer .
	///
	/// @remark This function detects creates a container of type @p _OutputContainer and uses a typical @c
	/// std::back_inserter or @c std::push_back_inserter to fill in elements as it is written to.
	//////
	template <typename _OutputContainer, typename _Input, typename _Encoding, typename _ErrorHandler, typename _State>
	constexpr auto decode(_Input&& __input, _Encoding&& __encoding, _ErrorHandler&& __error_handler, _State& __state) {
		using _UEncoding = __detail::__remove_cvref_t<_Encoding>;

		_OutputContainer __output {};
		if constexpr (__detail::__is_detected_v<__detail::__detect_adl_size, _Input>) {
			auto __output_size_hint = __detail::__adl::__adl_size(__input);
			__output_size_hint *= max_code_points_v<_UEncoding>;
			__output.reserve(__output_size_hint);
		}
		auto __insert_view     = unbounded_view(::std::back_inserter(__output));
		auto __stateful_result = decode_into(::std::forward<_Input>(__input), ::std::forward<_Encoding>(__encoding),
			::std::move(__insert_view), ::std::forward<_ErrorHandler>(__error_handler), __state);
		// We are explicitly discarding this information with this function call.
		(void)__stateful_result;
		return __output;
	}

	//////
	/// @brief Converts the code units of the given @p __input view through the encoding to code points the specified
	/// @p _OutputContainer type.
	///
	/// @tparam _OutputContainer The container type to serialize data into.
	///
	/// @param[in]     __input An input_view to read code units from and use in the decode operation that will
	/// produce code points.
	/// @param[in]     __encoding The encoding that will be used to decode the input's code points into
	/// output code units.
	/// @param[in]     __error_handler The error handlers for the from and to encodings,
	/// respectively.
	///
	/// @result An object of type @p _OutputContainer .
	///
	/// @remark This function creates a @c state using ztd::text::make_decode_state.
	//////
	template <typename _OutputContainer, typename _Input, typename _Encoding, typename _ErrorHandler>
	constexpr auto decode(_Input&& __input, _Encoding&& __encoding, _ErrorHandler&& __error_handler) {
		using _UEncoding = __detail::__remove_cvref_t<_Encoding>;
		using _State     = encoding_decode_state_t<_UEncoding>;

		_State __state = make_decode_state(__encoding);
		return decode<_OutputContainer>(::std::forward<_Input>(__input), ::std::forward<_Encoding>(__encoding),
			::std::forward<_ErrorHandler>(__error_handler), __state);
	}

	//////
	/// @brief Converts the code units of the given @p __input view through the encoding to code points the specified
	/// @p _OutputContainer type.
	///
	/// @tparam _OutputContainer The container type to serialize data into.
	///
	/// @param[in]     __input An input_view to read code units from and use in the decode operation that will
	/// produce code points.
	/// @param[in]     __encoding The encoding that will be used to decode the input's code points into
	/// output code units.
	///
	/// @result An object of type @p _OutputContainer .
	///
	/// @remark This function creates a @c handler using ztd::text::default_handler, but marks it as careless.
	//////
	template <typename _OutputContainer, typename _Input, typename _Encoding>
	constexpr auto decode(_Input&& __input, _Encoding&& __encoding) {
		__detail::__careless_handler __handler {};
		return decode<_OutputContainer>(
			::std::forward<_Input>(__input), ::std::forward<_Encoding>(__encoding), __handler);
	}

	//////
	/// @brief Converts the code units of the given @p __input view through the encoding to code points the specified
	/// @p _OutputContainer type.
	///
	/// @tparam _OutputContainer The container type to serialize data into.
	///
	/// @param[in]     __input An input_view to read code units from and use in the decode operation that will
	/// produce code points.
	///
	/// @result An object of type @p _OutputContainer .
	///
	/// @remark This function creates an @c encoding by using the @c value_type of the @p __input which is then passed
	/// through the ztd::text::default_code_point_encoding type to get the default desired encoding.
	//////
	template <typename _OutputContainer, typename _Input>
	constexpr auto decode(_Input&& __input) {
		using _UInput   = __detail::__remove_cvref_t<_Input>;
		using _Encoding = default_code_unit_encoding_t<__detail::__range_value_type_t<_UInput>>;
		return decode<_OutputContainer>(::std::forward<_Input>(__input), _Encoding {});
	}

	//////
	/// @brief Converts the code units of the given @p __input view through the encoding to code points in a @c
	/// std::vector or @c std::basic_string .
	///
	/// @param[in]     __input An input_view to read code units from and use in the decode operation that will
	/// produce code points.
	/// @param[in]     __encoding The encoding that will be used to decode the input's code points into
	/// output code units.
	/// @param[in]     __error_handler The error handlers for the from and to encodings,
	/// respectively.
	/// @param[in,out] __state A reference to the associated state for the @p __encoding 's decode step.
	///
	/// @result An object of type @c std::vector or @c std::basic_string , whichever is more appropriate for the output
	/// code unt type.
	///
	/// @remark This function detects creates a container of either @c std::Vector (when @c std::byte is involved) or
	/// @c std::basic_string and uses a typical @c std::back_inserter or @c std::push_back_inserter to fill in elements
	/// as it is written to.
	//////
	template <typename _Input, typename _Encoding, typename _ErrorHandler, typename _State>
	constexpr auto decode(_Input&& __input, _Encoding&& __encoding, _ErrorHandler&& __error_handler, _State& __state) {
		using _UEncoding = __detail::__remove_cvref_t<_Encoding>;
		using _CodePoint = encoding_code_point_t<_UEncoding>;
		using _OutputContainer
			= ::std::conditional_t<is_unicode_code_point_v<_CodePoint> || __detail::__is_character_v<_CodePoint>,
			     ::std::basic_string<_CodePoint>, ::std::vector<_CodePoint>>;

		return decode<_OutputContainer>(::std::forward<_Input>(__input), ::std::forward<_Encoding>(__encoding),
			::std::forward<_ErrorHandler>(__error_handler), __state);
	}

	//////
	/// @brief Converts the code units of the given @p __input view through the encoding to code points in a @c
	/// std::vector or @c std::basic_string .
	///
	/// @param[in]     __input An input_view to read code units from and use in the decode operation that will
	/// produce code points.
	/// @param[in]     __encoding The encoding that will be used to decode the input's code points into
	/// output code units.
	/// @param[in]     __error_handler The error handlers for the from and to encodings,
	/// respectively.
	///
	/// @result An object of type @c std::vector or @c std::basic_string , whichever is more appropriate for the output
	/// code unt type.
	///
	/// @remark This function creates a @c state using ztd::text::make_decode_state.
	//////
	template <typename _Input, typename _Encoding, typename _ErrorHandler>
	constexpr auto decode(_Input&& __input, _Encoding&& __encoding, _ErrorHandler&& __error_handler) {
		using _UEncoding = __detail::__remove_cvref_t<_Encoding>;
		using _State     = encoding_decode_state_t<_UEncoding>;

		_State __state = make_decode_state(__encoding);
		return decode(::std::forward<_Input>(__input), ::std::forward<_Encoding>(__encoding),
			::std::forward<_ErrorHandler>(__error_handler), __state);
	}

	//////
	/// @brief Converts the code units of the given @p __input view through the encoding to code points in a @c
	/// std::vector or @c std::basic_string .
	///
	/// @param[in]     __input An input_view to read code units from and use in the decode operation that will
	/// produce code points.
	/// @param[in]     __encoding The encoding that will be used to decode the input's code points into
	/// output code units.
	///
	/// @result An object of type @c std::vector or @c std::basic_string , whichever is more appropriate for the output
	/// code unt type.
	///
	/// @remark This function creates a @c handler using ztd::text::default_handler, but marks it as careless.
	//////
	template <typename _Input, typename _Encoding>
	constexpr auto decode(_Input&& __input, _Encoding&& __encoding) {
		__detail::__careless_handler __handler {};
		return decode(::std::forward<_Input>(__input), ::std::forward<_Encoding>(__encoding), __handler);
	}

	//////
	/// @brief Converts the code units of the given @p __input view through the encoding to code points in a @c
	/// std::vector or @c std::basic_string .
	///
	/// @param[in]     __input An input_view to read code units from and use in the decode operation that will
	/// produce code points.
	///
	/// @result An object of type @c std::vector or @c std::basic_string , whichever is more appropriate for the output
	/// code unt type.
	///
	/// @remark This function creates an @c encoding by using the @c value_type of the @p __input which is then passed
	/// through the ztd::text::default_code_point_encoding type to get the default desired encoding.
	//////
	template <typename _Input>
	constexpr auto decode(_Input&& __input) {
		using _UInput   = __detail::__remove_cvref_t<_Input>;
		using _Encoding = default_code_unit_encoding_t<__detail::__range_value_type_t<_UInput>>;
		return decode(::std::forward<_Input>(__input), _Encoding {});
	}

	//////
	/// @}
	//////

	ZTD_TEXT_INLINE_ABI_NAMESPACE_CLOSE_I_
}} // namespace ztd::text

#endif // ZTD_TEXT_DECODE_HPP
