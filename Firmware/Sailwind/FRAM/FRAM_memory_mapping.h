/*
 * FRAM_memory_mapping.h
 *
 *  Created on: 28 Sep 2023
 *      Author: nicof
 */

#ifndef FRAM_MEMORY_MAPPING_H_
#define FRAM_MEMORY_MAPPING_H_

/*Memory Region where Position Informations are saved*/
#define LINEAR_GUIDE_INFOS  0x0000

/*Memory Region where the used IP Address is saved*/
#define FRAM_IP_ADDRESS     0x0100
#define FRAM_IP_SET_DEFAULT_FLAG	0x0180

/*Memory Region where linear_guide configurations are saved*/
#define FRAM_MAX_RPM        0x0200
#define FRAM_MAX_DELTA      0x0210

#endif /* FRAM_MEMORY_MAPPING_H_ */
