
#include <cmath>

double Quantize(double d)
{
	if(std::abs(d) <= 100)
		return floor(0.5 + d);

	double scale = 1;
	while(std::abs(d) >= 1000) {
		scale *= 10;
		d *= 0.1;
	};
	scale = std::copysign(scale, d);
	d = std::abs(d);

	const double log_quant = 128;
	const double l = floor(0.5 + std::log10(d) * log_quant) / log_quant;
	return scale * floor(0.5 + pow(10.0, l));
};

#if 0
#include <iostream>
int main()
{
	for(double d = 20000; d <= 40000; d += 123)
	{
		std::cout << d << " => " << Quantize(d) << std::endl;
	};

	for(double d = 50; d <= 4000; d += 1)
	{
		std::cout << d << " => " << Quantize(d) << std::endl;
	};

	return 0;
};
#endif
