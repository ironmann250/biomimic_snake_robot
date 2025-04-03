/*
 * utils.c
 *
 *  Created on: May 10, 2024
 *      Author: stark
 */

double d_abs(double x)
{
    return x < 0 ? -x : x;
}

double abs_(double x)

{
	if (x<0.0)
	{
		return -x;
	}
	return x;
}

int i_abs(int x)
{
    return x < 0 ? -x : x;
}
