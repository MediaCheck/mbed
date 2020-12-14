/*
 * Copyright (c) 2017, Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef BYZANCE_UBLOX_PPP_H_
#define BYZANCE_UBLOX_PPP_H_
#include "mbed.h"
#include "AT_CellularDevice.h"
#include "CellularInformation.h"

namespace mbed {

class BYZANCE_UBLOX_PPP : public AT_CellularDevice {
public:
    BYZANCE_UBLOX_PPP(FileHandle *fh);
    virtual nsapi_error_t init();
    virtual nsapi_error_t hard_power_on();
    virtual nsapi_error_t is_ready();
    static void get_imsi(char *target, uint16_t len = MAX_IMSI_LENGTH+1);
    static void get_iccid(char *target, uint16_t len = MAX_ICCID_LENGTH);
protected:
	DigitalInOut _rst;
	static char _imsi[MAX_IMSI_LENGTH+1];		//MAX_IMSI_LENGTH does not contain termination zero len
	static char _iccid[MAX_ICCID_LENGTH];	//MAX_ICCID_LENGTH already contains len including termination zero 
};

} // namespace mbed

#endif // BYZANCE_UBLOX_PPP_H_
