/* +---------------------------------------------------------------------------+
   |                 The Mobile Robot Programming Toolkit (MRPT)               |
   |                                                                           |
   |                          http://www.mrpt.org/                             |
   |                                                                           |
   | Copyright (c) 2005-2013, Individual contributors, see AUTHORS file        |
   | Copyright (c) 2005-2013, MAPIR group, University of Malaga                |
   | Copyright (c) 2012-2013, University of Almeria                            |
   | All rights reserved.                                                      |
   |                                                                           |
   | Redistribution and use in source and binary forms, with or without        |
   | modification, are permitted provided that the following conditions are    |
   | met:                                                                      |
   |    * Redistributions of source code must retain the above copyright       |
   |      notice, this list of conditions and the following disclaimer.        |
   |    * Redistributions in binary form must reproduce the above copyright    |
   |      notice, this list of conditions and the following disclaimer in the  |
   |      documentation and/or other materials provided with the distribution. |
   |    * Neither the name of the copyright holders nor the                    |
   |      names of its contributors may be used to endorse or promote products |
   |      derived from this software without specific prior written permission.|
   |                                                                           |
   | THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS       |
   | 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED |
   | TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR|
   | PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE |
   | FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL|
   | DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR|
   |  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)       |
   | HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,       |
   | STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN  |
   | ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           |
   | POSSIBILITY OF SUCH DAMAGE.                                               |
   +---------------------------------------------------------------------------+ */
#ifndef  MRPT_STRING_UTILS_H
#define  MRPT_STRING_UTILS_H

#include <mrpt/utils/utils_defs.h>

namespace mrpt
{
	namespace system
	{
		/** \addtogroup string_manage String management and utilities (in #include <mrpt/system/string_utils.h>)
		  * \ingroup mrpt_base_grp
		  * @{ */

		/** An OS-independent method for tokenizing a string.
		  * The extra parameter "context" must be a pointer to a "char*" variable, which needs no initialization and is used to save information between calls to strtok.
		  * \sa system::tokenize
		  */
		char BASE_IMPEXP  *strtok( char *str, const char *strDelimit, char **context ) MRPT_NO_THROWS;

		/** Tokenizes a string according to a set of delimiting characters.
		  * Example:
		  * \code
		  std::vector<std::string>	tokens;
		  tokenize( " - Pepe-Er  Muo"," -",tokens);
		  * \endcode
		  *
		  *  Will generate 3 tokens:
		  *		- "Pepe"
		  *		- "Er"
		  *		- "Muo"
		  */
		void  BASE_IMPEXP tokenize(
			const std::string			&inString,
			const std::string			&inDelimiters,
			std::deque<std::string>		&outTokens ) MRPT_NO_THROWS;

		/** Tokenizes a string according to a set of delimiting characters.
		  * Example:
		  * \code
		  std::vector<std::string>	tokens;
		  tokenize( " - Pepe-Er  Muo"," -",tokens);
		  * \endcode
		  *
		  *  Will generate 3 tokens:
		  *		- "Pepe"
		  *		- "Er"
		  *		- "Muo"
		  */
		void BASE_IMPEXP  tokenize(
			const std::string			&inString,
			const std::string			&inDelimiters,
			std::vector<std::string>		&outTokens ) MRPT_NO_THROWS;

		/**  Removes leading and trailing spaces */
		std::string BASE_IMPEXP trim(const std::string &str);

		/** Returns a lower-case version of a string.
		  * \sa lowerCase  */
		std::string  BASE_IMPEXP upperCase(const std::string& str);

		/** Returns an upper-case version of a string.
		  * \sa upperCase  */
		std::string  BASE_IMPEXP lowerCase(const std::string& str);

		/** Decodes a UTF-8 string into an UNICODE string.
		  *  See http://en.wikipedia.org/wiki/UTF-8  and http://www.codeguru.com/cpp/misc/misc/multi-lingualsupport/article.php/c10451/.
		  */
		void BASE_IMPEXP decodeUTF8( const std::string &strUTF8, vector_word &out_uniStr );

		/** Encodes a 2-bytes UNICODE string into a UTF-8 string.
		  *  See http://en.wikipedia.org/wiki/UTF-8 and http://www.codeguru.com/cpp/misc/misc/multi-lingualsupport/article.php/c10451/.
		  */
        void BASE_IMPEXP encodeUTF8( const vector_word &input, std::string &output );

		/** Encode a sequence of bytes as a string in base-64.
		  * \sa decodeBase64  */
		void BASE_IMPEXP encodeBase64( const vector_byte &inputData,  std::string &outString );

		/** Decode a base-64 string into the original sequence of bytes.
		  * \sa encodeBase64
		  * \return false on invalid base-64 string passed as input, true on success.
		  */
		bool BASE_IMPEXP decodeBase64( const std::string &inString, vector_byte &outData );

		/** This function implements formatting with the appropriate SI metric unit prefix: 1e-12->'p', 1e-9->'n', 1e-6->'u', 1e-3->'m', 1->'', 1e3->'K', 1e6->'M', 1e9->'G', 1e12->'T'
		 * \sa intervalFormat */
		std::string BASE_IMPEXP unitsFormat(const double val,int nDecimalDigits=2, bool middle_space=true);

		/** Enlarge the string with spaces up to the given length. */
		std::string BASE_IMPEXP rightPad(const std::string &str, const size_t total_len, bool truncate_if_larger = false);

		/** Return true if the two strings are equal (case sensitive)  \sa strCmpI  */
		bool BASE_IMPEXP strCmp(const std::string &s1, const std::string &s2);

		/** Return true if the two strings are equal (case insensitive)  \sa strCmp */
		bool BASE_IMPEXP strCmpI(const std::string &s1, const std::string &s2);

		/** Return true if "str" starts with "subStr" (case sensitive)  \sa strStartsI  */
		bool BASE_IMPEXP strStarts(const std::string &str, const std::string &subStr);

		/** Return true if "str" starts with "subStr" (case insensitive)  \sa strStarts */
		bool BASE_IMPEXP strStartsI(const std::string &str, const std::string &subStr);

		/** Generates a string for a container in the format [A,B,C,...], and the fmt string for <b>each</b> vector element. */
		template <typename T>
		std::string sprintf_container(const char *fmt, const T &V )
		{
			std::string ret = "[";
			typename T::const_iterator it=V.begin();
			for (;it!=V.end();)
			{
				ret+= format(fmt,*it);
				++it;
				if (it!=V.end())
					ret+= ",";
			}
			ret+="]";
			return ret;
		}

		/** @} */
	} // End of namespace
} // End of namespace

#endif
