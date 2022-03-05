// C++ program to calculate Distance
// Between Two Points on Earth
#include <bits/stdc++.h>

// Utility function for
// converting degrees to radians
long double toRadians(const long double degree)
{
	// cmath library in C++
	// defines the constant
	// M_PI as the value of
	// pi accurate to 1e-30
	long double one_deg = (M_PI) / 180;
	return (one_deg * degree);
};

long double distance(long double lat1, long double long1,
					long double lat2, long double long2)
{
	// Convert the latitudes
	// and longitudes
	// from degree to radians.
	lat1 = toRadians(lat1);
	long1 = toRadians(long1);
	lat2 = toRadians(lat2);
	long2 = toRadians(long2);
	
	// Haversine Formula
	long double dlong = long2 - long1;
	long double dlat = lat2 - lat1;

	long double ans = std::pow(std::sin(dlat / 2), 2) +
						std::cos(lat1) * std::cos(lat2) *
						std::pow(std::sin(dlong / 2), 2);

	ans = 2 * std::asin(std::sqrt(ans));

	// Radius of Earth in
	// Kilometers, R = 6371
	// Use R = 3956 for miles
	long double R = 6371;
	
	// Calculate the result
	ans = ans * R;

	return ans;
};


// This code is contributed
// by Aayush Chaturvedi
