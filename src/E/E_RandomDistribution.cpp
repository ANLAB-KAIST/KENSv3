/*
 * E_RandomDistribution.cpp
 *
 *  Created on: 2014. 11. 4.
 *      Author: Keunhong Lee
 */


#include <E/E_Common.hpp>
#include <E/E_RandomDistribution.hpp>

namespace E
{

RandomDistribution::RandomDistribution() : engine(time(0))
{

}

RandomDistribution::RandomDistribution(UUID seed) : engine(seed)
{

}
RandomDistribution::~RandomDistribution()
{

}

std::list<Real> RandomDistribution::distribute(Size count, Real total)
{
	std::list<Real> temp;
	std::list<Real> ret;

	Real sum = 0;

	for(Size k = 0; k < count; k++)
	{
		Real val = this->nextDistribution(0,total);
		temp.push_back(val);
		sum += val;
	}

	Real regulation = total / sum;

	for(Real item : temp)
	{
		ret.push_back(item * regulation);
	}

	return ret;
}

UniformDistribution::UniformDistribution() : RandomDistribution()
{

}
UniformDistribution::UniformDistribution(UUID seed) : RandomDistribution(seed)
{

}

Real UniformDistribution::nextDistribution(Real min, Real max)
{
	std::uniform_real_distribution<Real> dist(min, max);
	return dist(engine);
}

Real MinDistribution::nextDistribution(Real min, Real max)
{
	return min;
}

Real MaxDistribution::nextDistribution(Real min, Real max)
{
	return max;
}

ExpDistribution::ExpDistribution(UUID seed, Real average) : RandomDistribution(seed)
{
	this->averageLocation = average;
}

ExpDistribution::ExpDistribution(Real average)
{
	this->averageLocation = average;
}

Real ExpDistribution::nextDistribution(Real min, Real max)
{
	Real lambda = ((Real)1.0) / ((max - min) * averageLocation);

	std::uniform_real_distribution<Real> dist(exp(-((Real)1.0 / averageLocation)), 1);

	return std::min(min + (-log(dist(engine)) / lambda), max);
}

Real LinearDistribution::nextDistribution(Real min, Real max)
{
	std::uniform_real_distribution<Real> dist(0, 1);
	Real uniform = dist(engine);
	return min + (max-min) * sqrt(uniform);
}

}


