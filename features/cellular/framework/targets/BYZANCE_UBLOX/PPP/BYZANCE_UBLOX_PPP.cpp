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

#include "BYZANCE_UBLOX_PPP.h"
#include "AT_CellularNetwork.h"

using namespace mbed;
using namespace events;

char BYZANCE_UBLOX_PPP::_imsi[MAX_IMSI_LENGTH+1] = "000000000000000";       //MAX_IMSI_LENGTH does not contain termination zero len
char BYZANCE_UBLOX_PPP::_iccid[MAX_ICCID_LENGTH] = "00000000000000000000";  //MAX_ICCID_LENGTH already contains len including termination zero 
//used config for  TARGET_UBLOX_C027 (SARA-G350)
static const intptr_t cellular_properties[AT_CellularBase::PROPERTY_MAX] = {
    AT_CellularNetwork::RegistrationModeDisable,// C_EREG
    AT_CellularNetwork::RegistrationModeLAC,    // C_GREG
    AT_CellularNetwork::RegistrationModeLAC,    // C_REG
    0,  // AT_CGSN_WITH_TYPE
    1,  // AT_CGDATA
    1,  // AT_CGAUTH
    1,  // AT_CNMI
    1,  // AT_CSMP
    1,  // AT_CMGF
    1,  // AT_CSDH
    1,  // PROPERTY_IPV4_STACK
    0,  // PROPERTY_IPV6_STACK
    0,  // PROPERTY_IPV4V6_STACK
    0,  // PROPERTY_NON_IP_PDP_TYPE
    1,  // PROPERTY_AT_CGEREP
};

BYZANCE_UBLOX_PPP::BYZANCE_UBLOX_PPP(FileHandle *fh) : AT_CellularDevice(fh), _rst(MBED_CONF_BYZANCE_UBLOX_PPP_RST)
{
    _rst.input();
    _rst = 1;
    AT_CellularBase::set_cellular_properties(cellular_properties);
}

nsapi_error_t BYZANCE_UBLOX_PPP::init()
{
    _at->lock();

    //disable possible RTS/CTS and echos and clear possible errors
    _at->cmd_start("AT&K0E0");
    _at->cmd_stop_read_resp();
    _at->clear_error();
    
    _at->cmd_start("AT");
    _at->cmd_stop_read_resp();

    _at->cmd_start("ATE0");
    _at->cmd_stop_read_resp();

    //enable error codes due to 3gpp
    _at->cmd_start("AT+CMEE=1");
    _at->cmd_stop_read_resp();

    _at->cmd_start("AT+UGPIOC=16,2"); // Set GPIO 16 (Network LED) to network mode
    _at->cmd_stop_read_resp();

    //fill static array with IMSI
    _at->cmd_start("AT+CIMI");
    _at->cmd_stop();
    _at->resp_start();
    _at->read_string(_imsi, MAX_IMSI_LENGTH + 1);
    _at->resp_stop();

    //fill static array with ICCID
    _at->cmd_start("AT+CCID?");
    _at->cmd_stop();
    _at->resp_start("+CCID:");
    _at->read_string(_iccid, MAX_ICCID_LENGTH);
    _at->resp_stop();

    _at->cmd_start("AT+CGMM"); // get version
    _at->cmd_stop();
    _at->resp_start("SARA-U2"); //check if it is SARA-U2 version
    bool setup_higher_baudrate = _at->info_resp();
    _at->resp_stop();
    if(setup_higher_baudrate){ 
        _at->cmd_start("AT+IPR=460800"); // setup higher baudrate on modem
        _at->cmd_stop_read_resp();
        // setup on our serial
        UARTSerial *pointer = (UARTSerial *)_at->get_file_handle();
        pointer->set_baud(460800);
        wait_ms(10);
    }
    nsapi_error_t err = _at->unlock_return_error();
    return  err;
}

nsapi_error_t BYZANCE_UBLOX_PPP::hard_power_on()
{
    _rst = 0;
    _rst.output();
    _rst = 0;
    ThisThread::sleep_for(200);
    UARTSerial *pointer = (UARTSerial *)_at->get_file_handle();
    pointer->set_baud(115200);
    _rst.input();
    _rst = 1;
    return NSAPI_ERROR_OK;
}

nsapi_error_t BYZANCE_UBLOX_PPP::is_ready()
{
    //disable possible RTS/CTS and echos and clear possible errors caused by echoing etc.
    _at->lock();
    _at->cmd_start("AT&K0E0");
    _at->cmd_stop_read_resp();
    _at->clear_error();


    // we need to do this twice because for example after data mode the first 'AT' command will give modem a
    // stimulus that we are back to command mode.    
    _at->cmd_start("AT");
    _at->cmd_stop_read_resp();

    nsapi_error_t err = _at->unlock_return_error();
    return err;
}

void BYZANCE_UBLOX_PPP::get_imsi(char *target, uint16_t len){
    strncpy(target, _imsi, len);
}

void BYZANCE_UBLOX_PPP::get_iccid(char *target, uint16_t len){
    strncpy(target, _iccid, len);   
}

#if MBED_CONF_BYZANCE_UBLOX_PPP_PROVIDE_DEFAULT

#if !NSAPI_PPP_AVAILABLE
#error Must define lwip.ppp-enabled
#endif

#include "UARTSerial.h"
CellularDevice *CellularDevice::get_default_instance()
{
    static UARTSerial serial(MBED_CONF_BYZANCE_UBLOX_PPP_TX, MBED_CONF_BYZANCE_UBLOX_PPP_RX, MBED_CONF_BYZANCE_UBLOX_PPP_BAUDRATE);
#if defined (MBED_CONF_UBLOX_AT_RTS) && defined(MBED_CONF_UBLOX_AT_CTS)
    tr_debug("BYZANCE_UBLOX_PPP flow control: RTS %d CTS %d", MBED_CONF_BYZANCE_UBLOX_PPP_RTS, MBED_CONF_BYZANCE_UBLOX_PPP_CTS);
    serial.set_flow_control(SerialBase::RTSCTS, MBED_CONF_BYZANCE_UBLOX_PPP_RTS, MBED_CONF_BYZANCE_UBLOX_PPP_CTS);
#endif
    static BYZANCE_UBLOX_PPP device(&serial);
    return &device;
}
#endif
