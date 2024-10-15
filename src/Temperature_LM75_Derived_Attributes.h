#include "sai_platform_globals.h"

#ifndef TEMPERATURE_LM75_DERIVED_ATTRIBUTES_H
#define TEMPERATURE_LM75_DERIVED_ATTRIBUTES_H

namespace sai
{
	struct RegisterLayout {
		uint8_t					temperature;
		uint8_t					configuration;
		uint8_t					temperature_low;
		uint8_t					temperature_high;
	};
	struct Attributes {
		uint8_t					temperature_width;
		uint8_t					default_temperature_resolution;
		uint8_t					default_temperature_frac_width;
		uint8_t					max_temperature_resolution;
		const RegisterLayout	* registers;
	};
	stxp	RegisterLayout	LM75_Compatible_Registers		=
		{ .temperature						= 0x00
		, .configuration					= 0x01
		, .temperature_low					= 0x02
		, .temperature_high					= 0x03
		};
	stxp	Attributes		Generic_LM75_Attributes			=
		{ .temperature_width				= 16
		, .default_temperature_resolution	= 9
		, .default_temperature_frac_width	= 8
		, .max_temperature_resolution		= 9
		, .registers						= &LM75_Compatible_Registers
		};
	stxp	Attributes		Generic_LM75_10Bit_Attributes	=
		{ .temperature_width				= 16
		, .default_temperature_resolution	= 10
		, .default_temperature_frac_width	= 8
		, .max_temperature_resolution		= 10
		, .registers						= &LM75_Compatible_Registers
		};
	stxp	Attributes		Generic_LM75_11Bit_Attributes	=
		{ .temperature_width				= 16
		, .default_temperature_resolution	= 11
		, .default_temperature_frac_width	= 8
		, .max_temperature_resolution		= 11
		, .registers						= &LM75_Compatible_Registers
		};
	stxp	Attributes		Generic_LM75_12Bit_Attributes	=
		{ .temperature_width				= 16
		, .default_temperature_resolution	= 12
		, .default_temperature_frac_width	= 8
		, .max_temperature_resolution		= 12
		, .registers						= &LM75_Compatible_Registers
		};
	stxp	Attributes		TI_TMP102_Attributes			=
		{ .temperature_width				= 16
		, .default_temperature_resolution	= 12
		, .default_temperature_frac_width	= 8
		, .max_temperature_resolution		= 13
		, .registers						= &LM75_Compatible_Registers
		};
} // namespace 

#endif // TEMPERATURE_LM75_DERIVED_ATTRIBUTES_H
