// RCopy.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include "Reserver.h"





int main(int argc, char* argv[])
{
	try
	{
		locale::global(locale(""));		

		if (argc > 1)
		{
			if (argv[1] == string("-schtask"))
			{
				cout << "Time for RCOPY" << endl;
				MessageBox(0,"Time for RCOPY","RCOPY",0);
			}
		}

		


		Reserver r;
		r.Process();

		cout << "Success finished!!! Press any key" << endl;
		char symbol;
		cin >> symbol;

		return 0;
	}
	catch (...)
	{
		cout << "Finished with error!!! Press any key" << endl;
		char symbol;
		cin >> symbol;
		return -1;
	} 
}


