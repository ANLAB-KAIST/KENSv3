/*
 * E_RandomDistribution.hpp
 *
 *  Created on: Nov 4, 2014
 *      Author: leeopop
 */

#ifndef E_RANDOMDISTRIBUTION_HPP_
#define E_RANDOMDISTRIBUTION_HPP_

#include <E/E_Common.hpp>

namespace E
{

class RandomDistribution
{
private:
	static UUID offset;
protected:
	std::default_random_engine engine;
public:
	RandomDistribution();
	RandomDistribution(UUID seed);
	virtual ~RandomDistribution();
	virtual Real nextDistribution(Real min, Real max) = 0;
	virtual std::list<Real> distribute(Size count, Real total) final;
};

class UniformDistribution : public RandomDistribution
{
public:
	UniformDistribution();
	UniformDistribution(UUID seed);
	virtual Real nextDistribution(Real min, Real max);
};

class MinDistribution : public RandomDistribution
{
public:
	virtual Real nextDistribution(Real min, Real max);
};

class MaxDistribution : public RandomDistribution
{
public:
	virtual Real nextDistribution(Real min, Real max);
};

class ExpDistribution : public RandomDistribution
{
private:
	Real averageLocation;
public:
	ExpDistribution(UUID seed, Real average = (Real)0.5);
	ExpDistribution(Real average = (Real)0.5);
	virtual Real nextDistribution(Real min, Real max);
};

class LinearDistribution : public RandomDistribution
{
public:
	virtual Real nextDistribution(Real min, Real max);
};

}

#endif /* E_RANDOMDISTRIBUTION_HPP_ */
