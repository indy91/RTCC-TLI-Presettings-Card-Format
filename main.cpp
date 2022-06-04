/***************************************************************************
  This file is part of Project Apollo - NASSP
  Copyright (c) 2022 Niklas Beug

  RTCC TLI presettings converter

  Project Apollo is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Project Apollo is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Project Apollo; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  See http://nassp.sourceforge.net/license/ for more details.

  **************************************************************************/

#include <iostream>
#include <fstream>
#include <string>

bool SearchForDoubleOpp(std::ifstream &file, const char *str, char opp, int num, double &val, double defval);
bool SearchForDoubleOpp2(std::ifstream &file, const char *str, char opp, double &val, double defval);
bool SearchForDouble(std::ifstream &file, const char *str, double &val, double defval);

int main()
{
	//INPUT PARAMETERS:

	//Day in year
	int LaunchDay = 207;
	//Input file name of scenario with LVDC data
	char FileNameIn[] = "Apollo 15 - Launch.scn";
	//Output file name of RTCC TLI parameters file, goes into \Config\ProjectApollo\RTCC
	//Contains punch card format from MSC internal note 69-FM-171
	//https://web.archive.org/web/20100524010957/http://ntrs.nasa.gov/archive/nasa/casi.ntrs.nasa.gov/19740072570_1974072570.pdf
	char FileNameOut[] = "1971-07-26 TLI.txt";


	char OppChar;
	int i, j, opp;
	char Buffer[128];

	double val1, val2, val3, val4;
	std::ifstream in;
	std::ofstream out;

	in.open(FileNameIn);
	out.open(FileNameOut);
	if (in.is_open() == false) return 0;

	const double PI = 3.14159265358979323846;
	const double RAD = PI / 180.0;

	opp = 1;
	OppChar = 'A';

	for (j = 0;j < 2;j++)
	{
		//Card 1, 24
		SearchForDoubleOpp(in, "LVDC_TP", OppChar, 0, val3, -1.0);
		SearchForDoubleOpp(in, "LVDC_CCS", OppChar, 0, val4, -1.0);
		snprintf(Buffer, 128, "%d %d %.3lf %.7lf", LaunchDay, opp, val3, val4);
		out << Buffer << "\n";
		//Card 2, 25
		SearchForDoubleOpp(in, "LVDC_C3", OppChar, 0, val1, -1.0);
		SearchForDoubleOpp(in, "LVDC_EN", OppChar, 0, val2, -1.0);
		SearchForDoubleOpp(in, "LVDC_RAS", OppChar, 0, val3, -1.0);
		SearchForDoubleOpp(in, "LVDC_DEC", OppChar, 0, val4, -1.0);
		snprintf(Buffer, 128, "%.1lf %.7lf %.7lf %.7lf", val1, val2, val3, val4);
		out << Buffer << "\n";

		for (i = 0;i < 7;i++)
		{
			//Card 3, 6...
			SearchForDoubleOpp(in, "LVDC_TP", OppChar, 2 * i + 1, val1, 1000.0);
			SearchForDoubleOpp(in, "LVDC_CCS", OppChar, 2 * i + 1, val2, 0.0);
			SearchForDoubleOpp(in, "LVDC_C3", OppChar, 2 * i + 1, val3, 0.0);
			SearchForDoubleOpp(in, "LVDC_EN", OppChar, 2 * i + 1, val4, 0.0);
			snprintf(Buffer, 128, "%.3lf %.7lf %.1lf %.7lf", val1, val2, val3, val4);
			out << Buffer << "\n";
			//Card 4, 7...
			SearchForDoubleOpp(in, "LVDC_RAS", OppChar, 2 * i + 1, val1, 0.0);
			SearchForDoubleOpp(in, "LVDC_DEC", OppChar, 2 * i + 1, val2, 0.0);
			SearchForDoubleOpp(in, "LVDC_TP", OppChar, 2 * i + 2, val3, 1000.0);
			SearchForDoubleOpp(in, "LVDC_CCS", OppChar, 2 * i + 2, val4, 0.0);
			snprintf(Buffer, 128, "%.7lf %.7lf %.3lf %.7lf", val1, val2, val3, val4);
			out << Buffer << "\n";
			//Card 5, 8...
			SearchForDoubleOpp(in, "LVDC_C3", OppChar, 2 * i + 2, val1, 0.0);
			SearchForDoubleOpp(in, "LVDC_EN", OppChar, 2 * i + 2, val2, 0.0);
			SearchForDoubleOpp(in, "LVDC_RAS", OppChar, 2 * i + 2, val3, 0.0);
			SearchForDoubleOpp(in, "LVDC_DEC", OppChar, 2 * i + 2, val4, 0.0);
			snprintf(Buffer, 128, "%.1lf %.7lf %.7lf %.7lf", val1, val2, val3, val4);
			out << Buffer << "\n";
		}

		opp = 2;
		OppChar = 'B';
	}

	//Cards 47 to 460 would contain the same data but for the 2nd to 10th launch day
	//The card format was slightly adapted to result in one text file for one launch day

	opp = 1;
	OppChar = 'A';

	for (j = 0;j < 2;j++)
	{
		//Card 461, 465
		SearchForDoubleOpp2(in, "LVDC_TST", OppChar, val3, 15000.0);
		SearchForDoubleOpp2(in, "LVDC_BETA", OppChar, val4, 61.89975);
		snprintf(Buffer, 128, "%d %d %.3lf %.7lf", LaunchDay, opp, val3, val4);
		out << Buffer << "\n";

		//Card 462, 466
		SearchForDoubleOpp2(in, "LVDC_ALFTS", OppChar, val1, 0.0);
		SearchForDoubleOpp2(in, "LVDC_F", OppChar, val2, 14.26968);
		SearchForDoubleOpp2(in, "LVDC_RN", OppChar, val3, 6575100.0);
		SearchForDoubleOpp2(in, "LVDC_T3PR", OppChar, val4, 310.8243);
		snprintf(Buffer, 128, "%.7lf %.7lf %.1lf %.4lf", val1, val2, val3, val4);
		out << Buffer << "\n";

		//Card 463, 467
		SearchForDoubleOpp2(in, "LVDC_TAU3R", OppChar, val1, 0.0);
		if (opp == 1)
		{
			SearchForDouble(in, "LVDC_T2IR", val2, 0.0);
		}
		else
		{
			SearchForDoubleOpp2(in, "LVDC_T2IR", OppChar, val2, 0.0);
		}
		
		SearchForDouble(in, "LVDC_V_ex2R", val3, 4221.827032);
		SearchForDouble(in, "LVDC_dotM_2R", val4, 215.2241);
		snprintf(Buffer, 128, "%.4lf %.2lf %.7lf %.7lf", val1, val2, val3, val4);
		out << Buffer << "\n";

		//Card 464, 468
		SearchForDoubleOpp2(in, "LVDC_DVBR", OppChar, val1, 3.7);
		SearchForDouble(in, "LVDC_tau2N", val2, 721.0);
		SearchForDouble(in, "LVDC_K_P1", val3, 0.0);
		SearchForDouble(in, "LVDC_K_Y1", val4, 0.0);
		snprintf(Buffer, 128, "%.7lf %.7lf %.1lf %.4lf", val1, val2, val3, val4);
		out << Buffer << "\n";

		opp = 2;
		OppChar = 'B';
	}

	//Cards 469 to 540 would contain the same data but for the 2nd to 10th launch day
	//The card format was slightly adapted to result in one text file for one launch day

	//Card 541
	SearchForDouble(in, "LVDC_T_LO", val2, 0.0);
	SearchForDouble(in, "LVDC_TETEO", val3, 0.0);
	SearchForDouble(in, "LVDC_omega_E", val4, 7.292107788e-5);
	snprintf(Buffer, 128, "%d %.3lf %.10lf %.15lf", LaunchDay, val2 + 17.0, val3, val4*3600.0);
	out << Buffer << "\n";

	//Card 542
	SearchForDouble(in, "LVDC_K_a1", val1, 0.0);
	SearchForDouble(in, "LVDC_K_a2", val2, 0.0);
	SearchForDouble(in, "LVDC_K_T3", val3, -.274);
	SearchForDouble(in, "LVDC_t_DS0", val4, 0.0);
	snprintf(Buffer, 128, "%.7lf %.7lf %.7lf %.3lf", val1, val2, val3, val4);
	out << Buffer << "\n";

	//Card 543
	SearchForDouble(in, "LVDC_t_DS1", val1, 10984.2);
	SearchForDouble(in, "LVDC_t_DS2", val2, 16503.1);
	SearchForDouble(in, "LVDC_t_DS3", val3, 0.0);
	SearchForDouble(in, "LVDC_hx[0][0]", val4, 0.0);
	snprintf(Buffer, 128, "%.3lf %.3lf %.3lf %.10lf", val1, val2, val3, val4*RAD);
	out << Buffer << "\n";

	//Card 544
	SearchForDouble(in, "LVDC_hx[0][1]", val1, 0.0);
	SearchForDouble(in, "LVDC_hx[0][2]", val2, 0.0);
	SearchForDouble(in, "LVDC_hx[0][3]", val3, 0.0);
	SearchForDouble(in, "LVDC_hx[0][4]", val4, 0.0);
	snprintf(Buffer, 128, "%.10lf %.10lf %.10lf %.10lf", val1*RAD, val2*RAD, val3*RAD, val4*RAD);
	out << Buffer << "\n";

	//Card 545
	SearchForDouble(in, "LVDC_t_D1", val1, 0.0);
	SearchForDouble(in, "LVDC_t_SD1", val2, 10984.2);
	SearchForDouble(in, "LVDC_hx[1][0]", val3, 0.0);
	SearchForDouble(in, "LVDC_hx[1][1]", val4, 0.0);
	snprintf(Buffer, 128, "%.3lf %.3lf %.7lf %.7lf", val1, val2, val3*RAD, val4*RAD);
	out << Buffer << "\n";

	//Card 546
	SearchForDouble(in, "LVDC_hx[1][2]", val1, 0.0);
	SearchForDouble(in, "LVDC_hx[1][3]", val2, 0.0);
	SearchForDouble(in, "LVDC_hx[1][4]", val3, 0.0);
	SearchForDouble(in, "LVDC_t_D2", val4, 10984.2);
	snprintf(Buffer, 128, "%.10lf %.10lf %.10lf %.3lf", val1*RAD, val2*RAD, val3*RAD, val4);
	out << Buffer << "\n";

	//Card 547
	SearchForDouble(in, "LVDC_t_SD2", val1, 5518.9);
	SearchForDouble(in, "LVDC_hx[2][0]", val2, 0.0);
	SearchForDouble(in, "LVDC_hx[2][1]", val3, 0.0);
	SearchForDouble(in, "LVDC_hx[2][2]", val4, 0.0);
	snprintf(Buffer, 128, "%.3lf %.10lf %.10lf %.10lf", val1, val2*RAD, val3*RAD, val4*RAD);
	out << Buffer << "\n";

	//Card 548
	SearchForDouble(in, "LVDC_hx[2][3]", val1, 0.0);
	SearchForDouble(in, "LVDC_hx[2][4]", val2, 0.0);
	SearchForDouble(in, "LVDC_t_D3", val3, 16503.1);
	SearchForDouble(in, "LVDC_t_SD3", val4, 1233.6);
	snprintf(Buffer, 128, "%.10lf %.10lf %.3lf %.3lf", val1*RAD, val2*RAD, val3, val4);
	out << Buffer;

	return 0;
}

bool SearchForDoubleOpp(std::ifstream &file, const char *str, char opp, int num, double &val, double defval)
{
	char Buff[128];
	snprintf(Buff, 128, "%s%c%d", str, opp, num);
	return SearchForDouble(file, Buff, val, defval);
}

bool SearchForDoubleOpp2(std::ifstream &file, const char *str, char opp, double &val, double defval)
{
	char Buff[128];
	snprintf(Buff, 128, "%s%c", str, opp);
	return SearchForDouble(file, Buff, val, defval);
}

bool SearchForDouble(std::ifstream &file, const char *str, double &val, double defval)
{
	char buffer[256];
	double e;
	std::string line;
	file.clear();
	file.seekg(0);

	while (std::getline(file, line))
	{
		if (sscanf_s(line.c_str(), "%s", buffer, 256) == 1) {
			if (!strcmp(buffer, str)) {
				if (sscanf_s(line.c_str(), "%s %lf", buffer, 256, &e) == 2) {
					val = e;
					return true;
				}
			}
		}
	}
	val = defval;
	return false;
}