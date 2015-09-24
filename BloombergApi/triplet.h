#pragma once

// triplet standard header

#include <iosfwd>

namespace stdext
{

	// TEMPLATE STRUCT triplet
	template<class _T1, class _T2, class _T3>
	struct triplet
	{
		typedef _T1 first_type;
		typedef _T2 second_type;
		typedef _T3 third_type;

		triplet()
			: first(_T1()), second(_T2()), third(_T3()) {}

		triplet(const _T1& _V1, const _T2& _V2, const _T3& _V3)
			: first(_V1), second(_V2), third(_V3) {}

		template<class U, class V, class W> triplet(const triplet<U, V, W> &p)
			: first(p.first), second(p.second), third(p.third) {}

		_T1 first;
		_T2 second;
		_T3 third;
	};

	template<class _T1, class _T2, class _T3> inline
		bool __cdecl operator==(const triplet<_T1, _T2, _T3>& _X,
			const triplet<_T1, _T2, _T3>& _Y)
		{return (_X.first == _Y.first && _X.second == _Y.second && _X.third == _Y.third); }
	template<class _T1, class _T2, class _T3> inline
		bool __cdecl operator!=(const triplet<_T1, _T2, _T3>& _X,
			const triplet<_T1, _T2, _T3>& _Y)
		{return (!(_X == _Y)); }
	template<class _T1, class _T2, class _T3> inline
		bool __cdecl operator<(const triplet<_T1, _T2, _T3>& _X,
			const triplet<_T1, _T2, _T3>& _Y)
		{return (_X.first < _Y.first ||
			!(_Y.first < _X.first) && _X.second < _Y.second ||
			!(_Y.first < _X.first) && !(_X.second < _Y.second) && _X.third < _Y.third); }
	template<class _T1, class _T2, class _T3> inline
		bool __cdecl operator>(const triplet<_T1, _T2, _T3>& _X,
			const triplet<_T1, _T2, _T3>& _Y)
		{return (_Y < _X); }
	template<class _T1, class _T2, class _T3> inline
		bool __cdecl operator<=(const triplet<_T1, _T2, _T3>& _X,
			const triplet<_T1, _T2, _T3>& _Y)
		{return (!(_Y < _X)); }
	template<class _T1, class _T2, class _T3> inline
		bool __cdecl operator>=(const triplet<_T1, _T2, _T3>& _X,
			const triplet<_T1, _T2, _T3>& _Y)
		{return (!(_X < _Y)); }
	template<class _T1, class _T2, class _T3> inline
		triplet<_T1, _T2, _T3> __cdecl make_triplet(const _T1& _X, const _T2& _Y, const _T3& _Z)
		{return (triplet<_T1, _T2, _T3>(_X, _Y, _Z)); }

} // namespace stdext
