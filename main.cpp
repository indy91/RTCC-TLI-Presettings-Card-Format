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
#include <vector>

bool SearchForDoubleOpp(std::ifstream &file, const char *str, char opp, int num, double &val, double defval);
bool SearchForDoubleOpp2(std::ifstream &file, const char *str, char opp, double &val, double defval);
bool SearchForDouble(std::ifstream &file, const char *str, double &val, double defval);
std::string FixedWidthString(std::string str, unsigned len);
std::string FormatID(std::string ID, int Opp, int Card);

void ReadSection1(const std::vector<std::string> &FileNameInArr, const std::vector<int> &LaunchDayArr, int Year, std::ofstream &out);
void ReadSection2(const std::vector<std::string> &FileNameInArr, const std::vector<int> &LaunchDayArr, int Year, std::ofstream &out);
void ReadSection3(const std::vector<std::string> &FileNameInArr, const std::vector<int> &LaunchDayArr, int Year, std::ofstream &out);

const double R_Earth = 6378165.0;
const double PI = 3.14159265358979323846;
const double RAD = PI / 180.0;
const double HRS = 3600.0;
const double ER2HR2ToM2SEC2 = pow(R_Earth / 3600.0, 2);
const double LBS = 0.45359237;
const double DT_GRR = 17.0;

int main()
{
	//INPUT PARAMETERS:
	int ApolloNo = 10;

	//Day in year
	std::vector<int> LaunchDayArr;
	//Year
	int Year;
	//Input file names of scenarios with LVDC data
	std::vector<std::string> FileNameInArr;
	//Output file name of RTCC TLI parameters file, goes into \Config\ProjectApollo\RTCC
	//Contains punch card format from MSC internal note 69-FM-171
	//https://web.archive.org/web/20100524010957/http://ntrs.nasa.gov/archive/nasa/casi.ntrs.nasa.gov/19740072570_1974072570.pdf
	char FileNameOut[128];

	if (ApolloNo == 8)
	{
		LaunchDayArr.push_back(356);
		Year = 1968;
		FileNameInArr.push_back("Apollo 8 - Launch.scn");
		snprintf(FileNameOut, 127, "Apollo 8 TLI.txt");
	}
	else if (ApolloNo == 10)
	{
		LaunchDayArr.push_back(138);
		Year = 1969;
		FileNameInArr.push_back("Apollo 10 - Launch.scn");
		snprintf(FileNameOut, 127, "Apollo 10 TLI.txt");
	}
	else if (ApolloNo == 11)
	{
		LaunchDayArr.push_back(197); FileNameInArr.push_back("Apollo 11 - Launch.scn");
		LaunchDayArr.push_back(199); FileNameInArr.push_back("Apollo 11 - July 18th Launch.scn");
		LaunchDayArr.push_back(202); FileNameInArr.push_back("Apollo 11 - July 21st Launch.scn");
		snprintf(FileNameOut, 127, "Apollo 11 TLI.txt");
		Year = 1969;
	}
	else if (ApolloNo == 12)
	{
		LaunchDayArr.push_back(318);
		Year = 1969;
		FileNameInArr.push_back("Apollo 12 - Launch.scn");
		snprintf(FileNameOut, 127, "Apollo 12 TLI.txt");
	}
	else if (ApolloNo == 13)
	{
		LaunchDayArr.push_back(101);
		Year = 1970;
		FileNameInArr.push_back("Apollo 13 - Launch.scn");
		snprintf(FileNameOut, 127, "Apollo 13 TLI.txt");
	}
	else if (ApolloNo == 14)
	{
		LaunchDayArr.push_back(31);
		Year = 1971;
		FileNameInArr.push_back("Apollo 14 - Launch.scn");
		snprintf(FileNameOut, 127, "Apollo 14 TLI.txt");
	}
	else if (ApolloNo == 15)
	{
		LaunchDayArr.push_back(207);
		Year = 1971;
		FileNameInArr.push_back("Apollo 15 - Launch.scn");
		snprintf(FileNameOut, 127, "Apollo 15 TLI.txt");
	}
	else if (ApolloNo == 16)
	{
		LaunchDayArr.push_back(107);
		Year = 1972;
		FileNameInArr.push_back("Apollo 16 - Launch.scn");
		snprintf(FileNameOut, 127, "Apollo 16 TLI.txt");
	}
	else if (ApolloNo == 17)
	{
		LaunchDayArr.push_back(342);
		Year = 1972;
		FileNameInArr.push_back("Apollo 17 - Launch.scn");
		snprintf(FileNameOut, 127, "Apollo 17 TLI.txt");
	}
	else return 0;

	std::ofstream out;

	out.open(FileNameOut);
	
	//Read first section
	ReadSection1(FileNameInArr, LaunchDayArr, Year, out);
	//Read second section
	ReadSection2(FileNameInArr, LaunchDayArr, Year, out);
	//Read third section
	ReadSection3(FileNameInArr, LaunchDayArr, Year, out);

	out.close();

	return 0;
}

void ReadSection1(const std::vector<std::string> &FileNameInArr, const std::vector<int> &LaunchDayArr, int Year, std::ofstream &out)
{
	//Cards 1 - 460

	std::ifstream in;
	std::string tempstr, columns[5], card, ID;
	char Buffer[128];
	char OppChar;
	double val1, val2, val3, val4;
	int cardnum, opp, LaunchDay;
	unsigned i, j, k;

	cardnum = 1;

	for (i = 0; i < FileNameInArr.size(); i++)
	{
		in.open(FileNameInArr[i]);
		if (in.is_open() == false) continue;

		LaunchDay = LaunchDayArr[i];

		//Set up ID
		snprintf(Buffer, 17, "%02d%03d", Year % 100, LaunchDay);
		ID.assign(Buffer);

		opp = 1;
		OppChar = 'A';

		for (j = 0; j < 2; j++)
		{
			//Card 1, 24
			SearchForDoubleOpp(in, "LVDC_TP", OppChar, 0, val3, -1.0);
			SearchForDoubleOpp(in, "LVDC_COS", OppChar, 0, val4, -1.0);

			tempstr = std::to_string(LaunchDay);
			columns[0] = FixedWidthString(tempstr, 17U);

			tempstr = std::to_string(opp);
			columns[1] = FixedWidthString(tempstr, 17U);

			snprintf(Buffer, 17, "%.8E", val3 / HRS);
			tempstr.assign(Buffer);
			columns[2] = FixedWidthString(tempstr, 17U);

			snprintf(Buffer, 17, "%.8E", val4);
			tempstr.assign(Buffer);
			columns[3] = FixedWidthString(tempstr, 17U);

			columns[4] = FormatID(ID, opp, cardnum);

			card = columns[0] + columns[1] + columns[2] + columns[3] + columns[4];
			out << card << std::endl;
			cardnum++;

			//Card 2, 25
			SearchForDoubleOpp(in, "LVDC_C3", OppChar, 0, val1, -1.0);
			SearchForDoubleOpp(in, "LVDC_EN", OppChar, 0, val2, -1.0);
			SearchForDoubleOpp(in, "LVDC_RAS", OppChar, 0, val3, -1.0);
			SearchForDoubleOpp(in, "LVDC_DEC", OppChar, 0, val4, -1.0);

			snprintf(Buffer, 17, "%.8E", val1 / ER2HR2ToM2SEC2);
			tempstr.assign(Buffer);
			columns[0] = FixedWidthString(tempstr, 17U);

			snprintf(Buffer, 17, "%.8E", val2);
			tempstr.assign(Buffer);
			columns[1] = FixedWidthString(tempstr, 17U);

			snprintf(Buffer, 17, "%.8E", val3 * RAD);
			tempstr.assign(Buffer);
			columns[2] = FixedWidthString(tempstr, 17U);

			snprintf(Buffer, 17, "%.8E", val4 * RAD);
			tempstr.assign(Buffer);
			columns[3] = FixedWidthString(tempstr, 17U);

			columns[4] = FormatID(ID, opp, cardnum);

			card = columns[0] + columns[1] + columns[2] + columns[3] + columns[4];
			out << card << std::endl;
			cardnum++;

			for (k = 0; k < 7; k++)
			{
				//Card 3, 6...
				SearchForDoubleOpp(in, "LVDC_TP", OppChar, 2 * k + 1, val1, 1000.0);
				SearchForDoubleOpp(in, "LVDC_COS", OppChar, 2 * k + 1, val2, 0.0);
				SearchForDoubleOpp(in, "LVDC_C3", OppChar, 2 * k + 1, val3, 0.0);
				SearchForDoubleOpp(in, "LVDC_EN", OppChar, 2 * k + 1, val4, 0.0);

				snprintf(Buffer, 17, "%.8E", val1 / HRS);
				tempstr.assign(Buffer);
				columns[0] = FixedWidthString(tempstr, 17U);

				snprintf(Buffer, 17, "%.8E", val2);
				tempstr.assign(Buffer);
				columns[1] = FixedWidthString(tempstr, 17U);

				snprintf(Buffer, 17, "%.8E", val3 / ER2HR2ToM2SEC2);
				tempstr.assign(Buffer);
				columns[2] = FixedWidthString(tempstr, 17U);

				snprintf(Buffer, 17, "%.8E", val4);
				tempstr.assign(Buffer);
				columns[3] = FixedWidthString(tempstr, 17U);

				columns[4] = FormatID(ID, opp, cardnum);

				card = columns[0] + columns[1] + columns[2] + columns[3] + columns[4];
				out << card << std::endl;
				cardnum++;

				//Card 4, 7...
				SearchForDoubleOpp(in, "LVDC_RAS", OppChar, 2 * k + 1, val1, 0.0);
				SearchForDoubleOpp(in, "LVDC_DEC", OppChar, 2 * k + 1, val2, 0.0);
				SearchForDoubleOpp(in, "LVDC_TP", OppChar, 2 * k + 2, val3, 1000.0);
				SearchForDoubleOpp(in, "LVDC_COS", OppChar, 2 * k + 2, val4, 0.0);


				snprintf(Buffer, 17, "%.8E", val1 * RAD);
				tempstr.assign(Buffer);
				columns[0] = FixedWidthString(tempstr, 17U);

				snprintf(Buffer, 17, "%.8E", val2 * RAD);
				tempstr.assign(Buffer);
				columns[1] = FixedWidthString(tempstr, 17U);

				snprintf(Buffer, 17, "%.8E", val3 / HRS);
				tempstr.assign(Buffer);
				columns[2] = FixedWidthString(tempstr, 17U);

				snprintf(Buffer, 17, "%.8E", val4);
				tempstr.assign(Buffer);
				columns[3] = FixedWidthString(tempstr, 17U);

				columns[4] = FormatID(ID, opp, cardnum);

				card = columns[0] + columns[1] + columns[2] + columns[3] + columns[4];
				out << card << std::endl;
				cardnum++;

				//Card 5, 8...
				SearchForDoubleOpp(in, "LVDC_C3", OppChar, 2 * k + 2, val1, 0.0);
				SearchForDoubleOpp(in, "LVDC_EN", OppChar, 2 * k + 2, val2, 0.0);
				SearchForDoubleOpp(in, "LVDC_RAS", OppChar, 2 * k + 2, val3, 0.0);
				SearchForDoubleOpp(in, "LVDC_DEC", OppChar, 2 * k + 2, val4, 0.0);

				snprintf(Buffer, 17, "%.8E", val1 / ER2HR2ToM2SEC2);
				tempstr.assign(Buffer);
				columns[0] = FixedWidthString(tempstr, 17U);

				snprintf(Buffer, 17, "%.8E", val2);
				tempstr.assign(Buffer);
				columns[1] = FixedWidthString(tempstr, 17U);

				snprintf(Buffer, 17, "%.8E", val3 * RAD);
				tempstr.assign(Buffer);
				columns[2] = FixedWidthString(tempstr, 17U);

				snprintf(Buffer, 17, "%.8E", val4 * RAD);
				tempstr.assign(Buffer);
				columns[3] = FixedWidthString(tempstr, 17U);

				columns[4] = FormatID(ID, opp, cardnum);

				card = columns[0] + columns[1] + columns[2] + columns[3] + columns[4];
				out << card << std::endl;
				cardnum++;
			}

			opp = 2;
			OppChar = 'B';
		}
	
		in.close();
	}
}

void ReadSection2(const std::vector<std::string> &FileNameInArr, const std::vector<int> &LaunchDayArr, int Year, std::ofstream &out)
{
	//Cards 460 - 540

	std::ifstream in;
	std::string tempstr, columns[5], card, ID;
	char Buffer[128];
	char OppChar;
	double val1, val2, val3, val4;
	int cardnum, opp, LaunchDay;
	unsigned i, j;

	cardnum = 461;

	for (i = 0; i < FileNameInArr.size(); i++)
	{
		in.open(FileNameInArr[i]);
		if (in.is_open() == false) continue;

		LaunchDay = LaunchDayArr[i];

		//Set up ID
		snprintf(Buffer, 17, "%02d%03d", Year % 100, LaunchDay);
		ID.assign(Buffer);

		opp = 1;
		OppChar = 'A';

		for (j = 0; j < 2; j++)
		{
			//Card 461, 465
			SearchForDoubleOpp2(in, "LVDC_TST", OppChar, val3, 15000.0);
			SearchForDoubleOpp2(in, "LVDC_BETA", OppChar, val4, 61.89975);


			tempstr = std::to_string(LaunchDay);
			columns[0] = FixedWidthString(tempstr, 17U);

			tempstr = std::to_string(opp);
			columns[1] = FixedWidthString(tempstr, 17U);

			snprintf(Buffer, 17, "%.8E", val3 / HRS);
			tempstr.assign(Buffer);
			columns[2] = FixedWidthString(tempstr, 17U);

			snprintf(Buffer, 17, "%.8E", val4*RAD);
			tempstr.assign(Buffer);
			columns[3] = FixedWidthString(tempstr, 17U);

			columns[4] = FormatID(ID, opp, cardnum);

			card = columns[0] + columns[1] + columns[2] + columns[3] + columns[4];
			out << card << std::endl;
			cardnum++;

			//Card 462, 466
			SearchForDoubleOpp2(in, "LVDC_ALFTS", OppChar, val1, 0.0);
			SearchForDoubleOpp2(in, "LVDC_F", OppChar, val2, 14.26968);
			SearchForDoubleOpp2(in, "LVDC_RN", OppChar, val3, 6575100.0);
			SearchForDoubleOpp2(in, "LVDC_T3PR", OppChar, val4, 310.8243);

			snprintf(Buffer, 17, "%.8E", val1* RAD);
			tempstr.assign(Buffer);
			columns[0] = FixedWidthString(tempstr, 17U);

			snprintf(Buffer, 17, "%.8E", val2* RAD);
			tempstr.assign(Buffer);
			columns[1] = FixedWidthString(tempstr, 17U);

			snprintf(Buffer, 17, "%.8E", val3 / R_Earth);
			tempstr.assign(Buffer);
			columns[2] = FixedWidthString(tempstr, 17U);

			snprintf(Buffer, 17, "%.8E", val4 / HRS);
			tempstr.assign(Buffer);
			columns[3] = FixedWidthString(tempstr, 17U);

			columns[4] = FormatID(ID, opp, cardnum);

			card = columns[0] + columns[1] + columns[2] + columns[3] + columns[4];
			out << card << std::endl;
			cardnum++;

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


			snprintf(Buffer, 17, "%.8E", val1 / HRS);
			tempstr.assign(Buffer);
			columns[0] = FixedWidthString(tempstr, 17U);

			snprintf(Buffer, 17, "%.8E", val2 / HRS);
			tempstr.assign(Buffer);
			columns[1] = FixedWidthString(tempstr, 17U);

			snprintf(Buffer, 17, "%.8E", val3 / R_Earth * 3600.0);
			tempstr.assign(Buffer);
			columns[2] = FixedWidthString(tempstr, 17U);

			snprintf(Buffer, 17, "%.8E", val4 / LBS * HRS);
			tempstr.assign(Buffer);
			columns[3] = FixedWidthString(tempstr, 17U);

			columns[4] = FormatID(ID, opp, cardnum);

			card = columns[0] + columns[1] + columns[2] + columns[3] + columns[4];
			out << card << std::endl;
			cardnum++;

			//Card 464, 468
			SearchForDoubleOpp2(in, "LVDC_DVBR", OppChar, val1, 3.7);
			SearchForDouble(in, "LVDC_tau2N", val2, 721.0);
			SearchForDouble(in, "LVDC_K_P1", val3, 0.0);
			SearchForDouble(in, "LVDC_K_Y1", val4, 0.0);

			snprintf(Buffer, 17, "%.8E", val1* HRS / R_Earth);
			tempstr.assign(Buffer);
			columns[0] = FixedWidthString(tempstr, 17U);

			snprintf(Buffer, 17, "%.8E", val2 / HRS);

			tempstr.assign(Buffer);
			columns[1] = FixedWidthString(tempstr, 17U);

			snprintf(Buffer, 17, "%.8E", val3);
			tempstr.assign(Buffer);
			columns[2] = FixedWidthString(tempstr, 17U);

			snprintf(Buffer, 17, "%.8E", val4);
			tempstr.assign(Buffer);
			columns[3] = FixedWidthString(tempstr, 17U);

			columns[4] = FormatID(ID, opp, cardnum);

			card = columns[0] + columns[1] + columns[2] + columns[3] + columns[4];
			out << card << std::endl;
			cardnum++;

			opp = 2;
			OppChar = 'B';
		}

		in.close();
	}
}

void ReadSection3(const std::vector<std::string> &FileNameInArr, const std::vector<int> &LaunchDayArr, int Year, std::ofstream &out)
{
	//Cards 541 - 620

	std::ifstream in;
	std::string tempstr, columns[5], card, ID;
	char Buffer[128];
	double val1, val2, val3, val4;
	int cardnum, LaunchDay, opp;
	unsigned i;

	cardnum = 541;
	opp = 2; //This section is actually opportunity independent

	for (i = 0; i < FileNameInArr.size(); i++)
	{
		in.open(FileNameInArr[i]);
		if (in.is_open() == false) continue;

		LaunchDay = LaunchDayArr[i];

		//Set up ID
		snprintf(Buffer, 17, "%02d%03d", Year % 100, LaunchDay);
		ID.assign(Buffer);

		//Card 541
		SearchForDouble(in, "LVDC_T_LO", val2, 0.0);
		SearchForDouble(in, "LVDC_THTEO", val3, 0.0);
		SearchForDouble(in, "LVDC_omega_E", val4, 7.292107788e-5);


		tempstr = std::to_string(LaunchDay);
		columns[0] = FixedWidthString(tempstr, 17U);

		snprintf(Buffer, 17, "%.8E", (val2 + DT_GRR) / HRS); //Presetting has GRR time, RTCC needs liftoff time
		tempstr.assign(Buffer);
		columns[1] = FixedWidthString(tempstr, 17U);

		snprintf(Buffer, 17, "%.8E", val3 + DT_GRR * val4); //Presetting has angle at GRR time, RTCC needs liftoff time
		tempstr.assign(Buffer);
		columns[2] = FixedWidthString(tempstr, 17U);

		snprintf(Buffer, 17, "%.8E", val4*HRS);
		tempstr.assign(Buffer);
		columns[3] = FixedWidthString(tempstr, 17U);

		columns[4] = FormatID(ID, opp, cardnum);

		card = columns[0] + columns[1] + columns[2] + columns[3] + columns[4];
		out << card << std::endl;
		cardnum++;

		//Card 542
		SearchForDouble(in, "LVDC_K_a1", val1, 0.0);
		SearchForDouble(in, "LVDC_K_a2", val2, 0.0);
		SearchForDouble(in, "LVDC_K_T3", val3, -.274);
		SearchForDouble(in, "LVDC_t_DS0", val4, 0.0);

		snprintf(Buffer, 17, "%.8E", val1*HRS);
		tempstr.assign(Buffer);
		columns[0] = FixedWidthString(tempstr, 17U);

		snprintf(Buffer, 17, "%.8E", val2*pow(HRS, 2));
		tempstr.assign(Buffer);
		columns[1] = FixedWidthString(tempstr, 17U);

		snprintf(Buffer, 17, "%.8E", val3);
		tempstr.assign(Buffer);
		columns[2] = FixedWidthString(tempstr, 17U);

		snprintf(Buffer, 17, "%.8E", val4*HRS);
		tempstr.assign(Buffer);
		columns[3] = FixedWidthString(tempstr, 17U);

		columns[4] = FormatID(ID, opp, cardnum);

		card = columns[0] + columns[1] + columns[2] + columns[3] + columns[4];
		out << card << std::endl;
		cardnum++;

		//Card 543
		SearchForDouble(in, "LVDC_t_DS1", val1, 10984.2);
		SearchForDouble(in, "LVDC_t_DS2", val2, 16503.1);
		SearchForDouble(in, "LVDC_t_DS3", val3, 0.0);
		SearchForDouble(in, "LVDC_hx[0][0]", val4, 0.0);


		snprintf(Buffer, 17, "%.8E", val1 / HRS);
		tempstr.assign(Buffer);
		columns[0] = FixedWidthString(tempstr, 17U);

		snprintf(Buffer, 17, "%.8E", val2 / HRS);
		tempstr.assign(Buffer);
		columns[1] = FixedWidthString(tempstr, 17U);

		snprintf(Buffer, 17, "%.8E", val3 / HRS);
		tempstr.assign(Buffer);
		columns[2] = FixedWidthString(tempstr, 17U);

		snprintf(Buffer, 17, "%.8E", val4*RAD);
		tempstr.assign(Buffer);
		columns[3] = FixedWidthString(tempstr, 17U);

		columns[4] = FormatID(ID, opp, cardnum);

		card = columns[0] + columns[1] + columns[2] + columns[3] + columns[4];
		out << card << std::endl;
		cardnum++;

		//Card 544
		SearchForDouble(in, "LVDC_hx[0][1]", val1, 0.0);
		SearchForDouble(in, "LVDC_hx[0][2]", val2, 0.0);
		SearchForDouble(in, "LVDC_hx[0][3]", val3, 0.0);
		SearchForDouble(in, "LVDC_hx[0][4]", val4, 0.0);

		snprintf(Buffer, 17, "%.8E", val1*RAD);
		tempstr.assign(Buffer);
		columns[0] = FixedWidthString(tempstr, 17U);

		snprintf(Buffer, 17, "%.8E", val2*RAD);
		tempstr.assign(Buffer);
		columns[1] = FixedWidthString(tempstr, 17U);

		snprintf(Buffer, 17, "%.8E", val3*RAD);
		tempstr.assign(Buffer);
		columns[2] = FixedWidthString(tempstr, 17U);

		snprintf(Buffer, 17, "%.8E", val4*RAD);
		tempstr.assign(Buffer);
		columns[3] = FixedWidthString(tempstr, 17U);

		columns[4] = FormatID(ID, opp, cardnum);

		card = columns[0] + columns[1] + columns[2] + columns[3] + columns[4];
		out << card << std::endl;
		cardnum++;

		//Card 545
		SearchForDouble(in, "LVDC_t_D1", val1, 0.0);
		SearchForDouble(in, "LVDC_t_SD1", val2, 10984.2);
		SearchForDouble(in, "LVDC_hx[1][0]", val3, 0.0);
		SearchForDouble(in, "LVDC_hx[1][1]", val4, 0.0);

		snprintf(Buffer, 17, "%.8E", val1 / HRS);
		tempstr.assign(Buffer);
		columns[0] = FixedWidthString(tempstr, 17U);

		snprintf(Buffer, 17, "%.8E", val2 / HRS);
		tempstr.assign(Buffer);
		columns[1] = FixedWidthString(tempstr, 17U);

		snprintf(Buffer, 17, "%.8E", val3*RAD);
		tempstr.assign(Buffer);
		columns[2] = FixedWidthString(tempstr, 17U);

		snprintf(Buffer, 17, "%.8E", val4*RAD);
		tempstr.assign(Buffer);
		columns[3] = FixedWidthString(tempstr, 17U);

		columns[4] = FormatID(ID, opp, cardnum);

		card = columns[0] + columns[1] + columns[2] + columns[3] + columns[4];
		out << card << std::endl;
		cardnum++;

		//Card 546
		SearchForDouble(in, "LVDC_hx[1][2]", val1, 0.0);
		SearchForDouble(in, "LVDC_hx[1][3]", val2, 0.0);
		SearchForDouble(in, "LVDC_hx[1][4]", val3, 0.0);
		SearchForDouble(in, "LVDC_t_D2", val4, 10984.2);

		snprintf(Buffer, 17, "%.8E", val1*RAD);
		tempstr.assign(Buffer);
		columns[0] = FixedWidthString(tempstr, 17U);

		snprintf(Buffer, 17, "%.8E", val2*RAD);
		tempstr.assign(Buffer);
		columns[1] = FixedWidthString(tempstr, 17U);

		snprintf(Buffer, 17, "%.8E", val3*RAD);
		tempstr.assign(Buffer);
		columns[2] = FixedWidthString(tempstr, 17U);

		snprintf(Buffer, 17, "%.8E", val4 / HRS);
		tempstr.assign(Buffer);
		columns[3] = FixedWidthString(tempstr, 17U);

		columns[4] = FormatID(ID, opp, cardnum);

		card = columns[0] + columns[1] + columns[2] + columns[3] + columns[4];
		out << card << std::endl;
		cardnum++;

		//Card 547
		SearchForDouble(in, "LVDC_t_SD2", val1, 5518.9);
		SearchForDouble(in, "LVDC_hx[2][0]", val2, 0.0);
		SearchForDouble(in, "LVDC_hx[2][1]", val3, 0.0);
		SearchForDouble(in, "LVDC_hx[2][2]", val4, 0.0);

		snprintf(Buffer, 17, "%.8E", val1 / HRS);
		tempstr.assign(Buffer);
		columns[0] = FixedWidthString(tempstr, 17U);

		snprintf(Buffer, 17, "%.8E", val2*RAD);
		tempstr.assign(Buffer);
		columns[1] = FixedWidthString(tempstr, 17U);

		snprintf(Buffer, 17, "%.8E", val3*RAD);
		tempstr.assign(Buffer);
		columns[2] = FixedWidthString(tempstr, 17U);

		snprintf(Buffer, 17, "%.8E", val4*RAD);
		tempstr.assign(Buffer);
		columns[3] = FixedWidthString(tempstr, 17U);

		columns[4] = FormatID(ID, opp, cardnum);

		card = columns[0] + columns[1] + columns[2] + columns[3] + columns[4];
		out << card << std::endl;
		cardnum++;

		//Card 548
		SearchForDouble(in, "LVDC_hx[2][3]", val1, 0.0);
		SearchForDouble(in, "LVDC_hx[2][4]", val2, 0.0);
		SearchForDouble(in, "LVDC_t_D3", val3, 16503.1);
		SearchForDouble(in, "LVDC_t_SD3", val4, 1233.6);

		snprintf(Buffer, 17, "%.8E", val1*RAD);
		tempstr.assign(Buffer);
		columns[0] = FixedWidthString(tempstr, 17U);

		snprintf(Buffer, 17, "%.8E", val2*RAD);
		tempstr.assign(Buffer);
		columns[1] = FixedWidthString(tempstr, 17U);

		snprintf(Buffer, 17, "%.8E", val3 / HRS);
		tempstr.assign(Buffer);
		columns[2] = FixedWidthString(tempstr, 17U);

		snprintf(Buffer, 17, "%.8E", val4 / HRS);
		tempstr.assign(Buffer);
		columns[3] = FixedWidthString(tempstr, 17U);

		columns[4] = FormatID(ID, opp, cardnum);

		card = columns[0] + columns[1] + columns[2] + columns[3] + columns[4];
		out << card << std::endl;
		cardnum++;

		in.close();
	}
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

std::string FixedWidthString(std::string str, unsigned len)
{
	if (str.length() > len) return str;

	std::string str2;
	for (unsigned i = 0; i < len - str.length(); i++)
	{
		str2.append(" ");
	}
	str2.append(str);
	return str2;
}

std::string FormatID(std::string ID, int Opp, int Card)
{
	char Buff[8];
	std::string tempstr;

	snprintf(Buff, 7, "%01d%03d", Opp, Card);
	tempstr.assign(Buff);

	return FixedWidthString(ID + tempstr, 12U);
}