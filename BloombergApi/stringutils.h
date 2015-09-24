#pragma once

inline
char* ltrim(char* s)
{
	char *s1 = s;
	while (isspace(*s1)) ++s1;
	return (char*) memmove((void*) s, (void*) s1, 1 + strlen(s1));
}

inline
char* rtrim(char* s)
{
	char *s1 = s;
	while (*s1 != '\0')
		++s1;
	--s1;
	while (s1 >= s && isspace(*s1))
		*s1-- = '\0';
	return s;
}

inline
char* trim(char* s)
{
	return ltrim(rtrim(s));
}

inline
char* compressws(char* dest, const char* src)
{
	char *s = dest;
	bool was_ws = false;
	while (*src)
	{
		bool is_ws = (isspace(*src) != 0);

		if (!is_ws || (is_ws && !was_ws))
			*dest++ = *src;

		was_ws = is_ws;

		++src;
	}
	*dest = *src;

	return s;
}

template <typename T>
inline std::vector<std::basic_string<T> > split(const std::basic_string<T>& s, const T& c)
{
	std::vector<std::basic_string<T> > vec;
	std::basic_string<T>::const_iterator start = s.begin();
	std::basic_string<T>::const_iterator next = s.begin();
	while (next != s.end())
	{
		if (*next == c)
		{
			std::basic_string<T> s1(start, next);
			vec.push_back(s1);
			start = next + 1;
		}
		++next;
	}
	if (start != next || *(next-1) == c)
	{
		std::basic_string<T> s1(start, next);
		vec.push_back(s1);
	}

	return vec;
}
