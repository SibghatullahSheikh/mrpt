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
   +---------------------------------------------------------------------------+ */

#include <mrpt/base.h>
#include <mrpt/hwdrivers/CNationalInstrumentsDAQ.h>

using namespace mrpt;
using namespace mrpt::utils;
using namespace mrpt::slam;
using namespace mrpt::hwdrivers;
using namespace std;

// ------------------------------------------------------
//				Test_NIDAQ
// ------------------------------------------------------
void Test_NIDAQ()
{
	CNationalInstrumentsDAQ	 daq;

	// Load config:
#if 0
	//daq.loadConfig( CConfigFile( "./DAQ_example.ini") ,"DAQ1" );
#else
	// Or set params programatically:

	// Define a task with analog inputs:
	CNationalInstrumentsDAQ::TaskDescription task;
	task.has_ai = true;
	task.ai.physicalChannel = "Dev1/ai0:7";
	task.ai.physicalChannelCount = 8; // Must be the number of channels encoded in the "physicalChannel" string.
	task.ai.terminalConfig  = "DAQmx_Val_RSE";
	task.ai.minVal = -10;
	task.ai.maxVal =  10;

	daq.task_definitions.push_back(task);

#endif

	printf("[Example] Initializing DAQ...\n");
	daq.initialize();
	printf("[Example] Init passed.\n");

	printf("\n ** Press any key to stop grabbing ** \n");

	while (!mrpt::system::os::kbhit())
	{
		std::vector<mrpt::slam::CObservationRawDAQPtr> readObs;
		bool hardError;

		try
		{
			daq.readFromDAQ( readObs, hardError );
		}
		catch (std::exception &e)
		{
			cerr << e.what() << endl;
			hardError = true;
		}

		if (hardError)
			printf("[TEST] Hardware error=true!!\n");

		if (!readObs.empty())
		{
			// Look for analog readings:
			for (size_t i=0;i<readObs.size();i++)
			{
				if (readObs[i]->AIN_double.empty())
					continue; // Skip

				const size_t nSamplPerChan = readObs[i]->AIN_double.size() / readObs[i]->AIN_channel_count;
				cout << "Read " << nSamplPerChan << " samples. a[0]=" << readObs[i]->AIN_double[0] << endl;
			}
		}

		mrpt::system::sleep(1);
	};
}

int main()
{
	try
	{
		Test_NIDAQ();
		return 0;

	} catch (std::exception &e)
	{
		std::cout << "EXCEPTION: " << e.what() << std::endl;
		return -1;
	}
}

