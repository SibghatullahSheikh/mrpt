/* +---------------------------------------------------------------------------+
   |                 The Mobile Robot Programming Toolkit (MRPT)               |
   |                                                                           |
   |                          http://www.mrpt.org/                             |
   |                                                                           |
   | Copyright (c) 2005-2013, Individual contributors, see AUTHORS file        |
   | Copyright (c) 2005-2013, MAPIR group, University of Malaga                |
   | Copyright (c) 2012-2013, University of Almeria                            |
   | All rights reserved.                                                      |
   |                                                                           |
   | Redistribution and use in source and binary forms, with or without        |
   | modification, are permitted provided that the following conditions are    |
   | met:                                                                      |
   |    * Redistributions of source code must retain the above copyright       |
   |      notice, this list of conditions and the following disclaimer.        |
   |    * Redistributions in binary form must reproduce the above copyright    |
   |      notice, this list of conditions and the following disclaimer in the  |
   |      documentation and/or other materials provided with the distribution. |
   |    * Neither the name of the copyright holders nor the                    |
   |      names of its contributors may be used to endorse or promote products |
   |      derived from this software without specific prior written permission.|
   |                                                                           |
   | THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS       |
   | 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED |
   | TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR|
   | PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE |
   | FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL|
   | DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR|
   |  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)       |
   | HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,       |
   | STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN  |
   | ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           |
   | POSSIBILITY OF SUCH DAMAGE.                                               |
   |     You should have received a copy of the GNU General Public License     |
   |     along with MRPT.  If not, see <http://www.gnu.org/licenses/>.         |
   |                                                                           |
   +---------------------------------------------------------------------------+ */

#include <mrpt/hwdrivers.h> // Precompiled headers

#include <mrpt/hwdrivers/CLMS100eth.h>
#include <mrpt/system/string_utils.h>

#include <iostream>
#include <sstream>
#include <string.h>

#define APPERTURE           4.712385    // in radian <=> 270°

using namespace mrpt;
using namespace mrpt::system;
using namespace mrpt::utils;
using namespace mrpt::hwdrivers;
using namespace std;


IMPLEMENTS_GENERIC_SENSOR(CLMS100Eth,mrpt::hwdrivers)

CLMS100Eth::CLMS100Eth(string _ip, unsigned int _port):
        m_ip(_ip),
        m_port(_port),
		m_client(),
        m_turnedOn(false),
        m_cmd(),
        m_connected(false),
		m_sensorPose(0.0, 0.0, 0.0, 0.0, 0.0, 0.0),
        m_maxRange(20.0),
        m_beamApperture(.25*M_PI/180.0)
{
}

CLMS100Eth::~CLMS100Eth()
{
    if(m_connected)
        m_client.close();
//    delete m_client;
//    delete m_sensorPose;
}

void CLMS100Eth::initialize()
{
	if(!checkIsConnected())
	{
		THROW_EXCEPTION("Can't connect to LMS100 Ethernet Sensor check your configuration file.");
	}
	turnOn();
}

void CLMS100Eth::loadConfig_sensorSpecific( const mrpt::utils::CConfigFileBase &configSource,
                             const std::string	  &iniSection )
{
    loadExclusionAreas(configSource, iniSection);
    float pose_x, pose_y, pose_z, pose_yaw, pose_pitch, pose_roll;

    pose_x = configSource.read_float(iniSection,"pose_x",0,false);
    pose_y = configSource.read_float(iniSection,"pose_y",0,false);
    pose_z = configSource.read_float(iniSection,"pose_z",0,false);
    pose_yaw = configSource.read_float(iniSection,"pose_yax",0,false);
    pose_pitch = configSource.read_float(iniSection,"pose_pitch",0,false);
    pose_roll = configSource.read_float(iniSection,"pose_roll",0,false);
	m_ip = configSource.read_string(iniSection, "ip_address", "192.168.0.1", false);
	m_port = configSource.read_int(iniSection, "TCP_port", 2111, false);
	m_process_rate = configSource.read_int(iniSection, string("process_rate"), 10, false);
	m_sensorLabel = configSource.read_string(iniSection, "sensorLabel", "SICK", false);
    m_sensorPose = CPose3D( pose_x, pose_y, pose_z,
        DEG2RAD( pose_yaw ),DEG2RAD( pose_pitch ), DEG2RAD( pose_roll ));

}

bool CLMS100Eth::checkIsConnected(void)
{
    if(m_connected) return true;
    else
    {
        try{
            m_client.connect(m_ip, m_port);
        }catch(std::exception &e)
        {
            printf_debug(e.what());
            printf_debug("[CLMS100ETH] ERROR TRYING TO OPEN Ethernet DEVICE");
            return false;
        }
    }
    m_connected = true;
    return true;
}

bool CLMS100Eth::turnOff()
{
    if(m_client.isConnected())
        m_client.close();
    m_connected = false;
    m_turnedOn = false;
    return true;
}

bool CLMS100Eth::turnOn()
{
    /** From the LMS100 datasheet : :
     * * Login sMN SetAccessMode 03 F4724744F4724744
     * * Set Scanarea and Resolution
     * * sMN mLMPsetscancfg
     * * SWN LMDscandatacfg 01 00 0 1 0 00 00 0 0 0 0 1
     * * SMN mEEwriteall   Je ne le fais pas, car ca écrit en mémoire non volatile...
     * * Request scan : sRN LMDscandata OR sEN LMDscandata
     */
    size_t read;
    if(checkIsConnected())
    {
        try{
            {
                char msg[] = {"sMN SetAccessMode 03 F4724744"};
                char msgIn[100];
                sendCommand(msg);

                read = m_client.readAsync(msgIn, 100, 1000, 1000);  //18
                
                msgIn[read-1] = 0;
                printf_debug("read : %d\n",read);
                printf_debug("message : %s\n",string(&msgIn[1]).c_str());
                
                if(!read) return false;
            }
            {
                char msg[] = {"sMN mLMPsetscancfg +2500 +1 +2500 -450000 +2250000"};
                char msgIn[100];
                sendCommand(msg);

                read = m_client.readAsync(msgIn, 100, 1000, 1000);

                msgIn[read-1] = 0;
                printf_debug("read : %d\n",read);
                printf_debug("message : %s\n",string(&msgIn[1]).c_str());

                if(!read) return false;
            }
            {
                char msg[] = {"sWN LMDscandatacfg 01 00 0 1 0 00 00 0 0 0 0 +1"};
                char msgIn[100];
                sendCommand(msg);

                read = m_client.readAsync(msgIn, 100, 1000, 1000);

                msgIn[read-1] = 0;
                printf_debug("read : %d\n",read);
                printf_debug("message : %s\n",string(&msgIn[1]).c_str());

                if(!read) return false;
            }
            {
                char msg[] = {"sMN LMCstartmeas"};
                char msgIn[100];
                sendCommand(msg);
                read = m_client.readAsync(msgIn, 100, 1000, 1000);
                
                msgIn[read-1] = 0;
                printf_debug("message : %s\n",string(&msgIn[1]).c_str());
                if(!read) return false;
            }
            {
                char msgIn[100];
                char msg[] = {"sRN STlms"};
                do{
                    sendCommand(msg);
                    read = m_client.readAsync(msgIn, 100, 1000, 1000);
                    sleep(10000);
                    
                    msgIn[read-1] = 0;
                    printf_debug("message : %s\n",&msgIn[1]);
                    printf_debug("%c\n", msgIn[11]);
                    if(!read) return false;
                } while(msgIn[11] != '7');
            }
            m_turnedOn = true;
        }catch(std::exception &e)
        {
            printf_debug(e.what());
            return false;
        }
    }else
    {
        return false;
    }
    return true;
}

void CLMS100Eth::sendCommand(const char *cmd)
{
    generateCmd(cmd);
    if (!m_cmd.empty()) // one never knows...
		m_client.writeAsync(&m_cmd[0], m_cmd.size());
}

/** Add the start and end character.
 */
void CLMS100Eth::generateCmd(const char *cmd)
{
    if(strlen(cmd) > 995)
    {
        printf_debug("la commande est trop longue\n");
        return;
    }
    m_cmd = format("%c%s%c",0x02,cmd,0x03);
}


bool CLMS100Eth::decodeScan(char* buff, CObservation2DRangeScan& outObservation)
{
    char *next;
    unsigned int idx = 0;
    unsigned int scanCount=0;
    char* tmp;
    //double factor;

    next = strtok(buff, " ", &tmp);

    while(next && scanCount==0)
    {

        //cout << "Interpreting : " << next << endl;
        switch(++idx)
        {
        case 1:
            if(strncmp(&next[1], "sRA", 3) && strncmp(&next[1], "sSN", 3)) return false;
            break;
        case 2 :
            if(strcmp(next, "LMDscandata")) return false;
            break;
        case 6 :
            if(!strcmp(next, "1"))
            {
                THROW_EXCEPTION("STATUS error on LMS100");
                return false;
            }
			else 
			if(!strcmp(next, "4"))
			{
				THROW_EXCEPTION("Contamination error on LMS100");
				return false;
			}
            printf_debug("STATUS Ok.\n");
            break;
        case 21 :
            if(strcmp(next, "DIST1"))
            {
                THROW_EXCEPTION("LMS100 is not configured to send distances.");
                return false;
            }
            printf_debug("Distance : OK\n");
            break;
        case 22 :
            //factor = strtod(next, NULL);
            break;
        case 26 :
            scanCount = strtoul(next, NULL, 16);
            printf_debug("Scan Count : %d\n", scanCount);
            break;
        default :
                break;
    }
        next = strtok(NULL, " ", &tmp);
    }
    outObservation.aperture = APPERTURE;
    outObservation.rightToLeft = false;
    outObservation.stdError = 0.012f;
    outObservation.sensorPose = m_sensorPose;
    outObservation.beamAperture = m_beamApperture;
    outObservation.maxRange = m_maxRange;
	outObservation.timestamp				= mrpt::system::getCurrentTime();
	outObservation.sensorLabel             = m_sensorLabel;

	outObservation.scan.clear();
	outObservation.validRange.clear();
    unsigned int i;
    for(i = 0 ; i < scanCount && next; i++, next = strtok(NULL, " ", &tmp))
    {
        outObservation.scan.push_back(double(strtoul(next, NULL, 16))/1000.0);
        outObservation.validRange.push_back(outObservation.scan[i] <= outObservation.maxRange);
    }
    return i>=scanCount;
}

void CLMS100Eth::doProcessSimple(bool &outThereIsObservation, CObservation2DRangeScan &outObservation, bool &hardwareError)
{
    if(!m_turnedOn)
    {
        hardwareError = true;
        outThereIsObservation = false;
        return;
    }
    hardwareError = false;

    char msg[] = {"sRN LMDscandata"};
    sendCommand(msg);
    char buffIn[16*1024];
    //size_t read = m_client.readAsync(buffIn, sizeof(buffIn), 100, 100);
    //cout << "read :" << read << endl;
    //while(m_client.readAsync(buffIn, sizeof(buffIn), 100, 100)) cout << "Lit dans le vent" << endl;
    
    m_client.readAsync(buffIn, sizeof(buffIn), 40, 40);

    if(decodeScan(buffIn, outObservation))
    {
        // Do filter:
        this->filterByExclusionAreas( outObservation );
        this->filterByExclusionAngles( outObservation );
        outThereIsObservation = true;
        hardwareError = false;
    }else
    {
        hardwareError = true;
        outThereIsObservation = false;
        printf_debug("doProcessSimple failed\n");
    }
}

/*-------------------------------------------------------------*/
void CLMS100Eth::doProcess( )
{
	CObservation2DRangeScanPtr obs= CObservation2DRangeScan::Create();
	bool isThereObservation, hwError;
	try
	{
		doProcessSimple(isThereObservation, *obs, hwError);
		if(hwError) m_state = ssError;
		else m_state = ssWorking;
		// if at least one data have been sensed :
		if(isThereObservation)
		{
			appendObservation( obs );
		}
	}
	catch(...)
	{
		m_state = ssError;
	    THROW_EXCEPTION("No observation received from the Phidget board!");
	}
}

/** A method to set the sensor pose on the robot.
 */
void CLMS100Eth::setSensorPose(const CPose3D& _pose)
{
	m_sensorPose = _pose;
}
