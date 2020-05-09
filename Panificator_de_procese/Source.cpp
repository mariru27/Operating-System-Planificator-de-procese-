#include <iostream>
#include <queue>
#include <ctime>
#include <random>
#include <list>
#include <chrono>


#define MEMORIE_TOTALA

class Partitie
{
public:
	int memorieOcupata;
	int adresaInceput;
	int adresaSfarsit;
	int dimensiune_partitie; //biti
	Partitie(int memorieOcupata, int dimensiune_partitie) : memorieOcupata(memorieOcupata), dimensiune_partitie(dimensiune_partitie) 
	{
		adresaInceput = 0;
		adresaSfarsit = 0;
	}
	Partitie()
	{
		memorieOcupata = 0;
	}
};

class Proces 
{
public:
	Partitie* partitieProprie;
	int durataViata;
	int memorieNecesara;
	int itimp; //intervarul de timp intre momentul la care este incarcat in memorie procesul P(i-1)
			   //si momentul la care este incarcat in memorie procesul P(i)
	int mtimp; //momentul de timp la care a fost incarcat in memorie procesul
	int timpFolosireProcesor;
	int timpTerminareProces;
	Proces(int durataViata = 0, int memorieNecesara = 0, int itimp = 0, int mtimp = 0) : durataViata(durataViata), 
		memorieNecesara(memorieNecesara), itimp(itimp), mtimp(mtimp) 
	{
		timpFolosireProcesor = 0;
		timpTerminareProces = 0;
	}
	bool ProcesTerminat()
	{
		return durataViata <= timpFolosireProcesor;
	}

};

#define NUMBER 100 //Numarul de procese
const int T = 90000; //durata simularii
const int q = 60; //cuanta de timp 50 secunde
const int Dmin = 20, Dmax = 20000, Mmin = 25, Mmax = 800, Imin = 1, Imax = 400;
std::list<Partitie> memorieLibera, memoriePlina; //vom gestiona memoria ocupata de procese cu ajutorul listelor

std::queue<Proces> coadaDeProcese;
std::vector<Proces> procese;
std::vector<Proces> rezultateProcese;

void GenerareValori()
{
	for (std::size_t it = 0; it < NUMBER; ++it)
	{
		unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();

		std::default_random_engine generate(seed);
		std::uniform_int_distribution<int> durataViata(Dmin, Dmax);
		std::uniform_int_distribution<int> memorieNecesara(Mmin, Mmax);
		std::uniform_int_distribution<int> mtimp(Imin, Imax); // momentul de tip in care a fost incarcat in  memorie procesul

		procese.emplace_back(Proces(durataViata(generate), memorieNecesara(generate), mtimp(generate), mtimp(generate)));
	}
	   
}

void ReconstitueMemorieLibera()
{
	//daca se poate se va reconstitui memoria libera
	std::list<Partitie>::iterator itUrm;
	memorieLibera.sort([](Partitie& a, Partitie& b)->bool
	{
		return a.adresaInceput < b.adresaInceput;
	});
	itUrm = memorieLibera.end();
	for (auto it = memorieLibera.begin(); it != memorieLibera.end(); ++it)
	{
		if (itUrm != memorieLibera.end() && itUrm->adresaSfarsit+1 == it->adresaInceput)
		{
			it->adresaInceput = itUrm->adresaInceput;
			it->dimensiune_partitie = it->adresaSfarsit - it->adresaInceput;
			memorieLibera.erase(itUrm);
		}
		itUrm = it;

	}

}



// o sa folosesc liste pentru gestionarea memoriei ocupata de procese;
// memoria initiala(libera) = 40000 biti
bool RezervaMemorie(Proces& p)
{
	std::list<Partitie>::iterator freeParition = memorieLibera.end();
	memorieLibera.sort([](Partitie& a, Partitie& b)->bool
	{
		return a.dimensiune_partitie < b.dimensiune_partitie;
	});
	for (auto it = memorieLibera.begin(); it != memorieLibera.end(); ++it)
	{
		if (it->dimensiune_partitie >= (p.memorieNecesara+1))
		{
			freeParition = it;
			break;
		}
	}
	//daca nu o sa mai gasim memorie libera, acel proces va astepta pana se va elibera memoria
	if (freeParition != memorieLibera.end())
	{
		Partitie* partitieNoua = new Partitie;
		partitieNoua->adresaInceput = freeParition->adresaInceput;
		freeParition->adresaInceput += p.memorieNecesara;
		partitieNoua->adresaSfarsit = freeParition->adresaInceput;
		++(freeParition->adresaInceput);
		p.partitieProprie = partitieNoua;
		partitieNoua->dimensiune_partitie = partitieNoua->adresaSfarsit - partitieNoua->adresaInceput;
		freeParition->dimensiune_partitie = freeParition->adresaSfarsit - freeParition->adresaInceput;
		memoriePlina.push_back(*partitieNoua);

		ReconstitueMemorieLibera();
		return true;
	}
	
	return false;
}

void ElibereazaMemorie(Proces& p)
{
	Partitie* partitieNoua = new Partitie;
	partitieNoua = p.partitieProprie;
	partitieNoua->memorieOcupata = 0;
	for (auto it = memoriePlina.begin(); it != memoriePlina.end(); ++it)
	{
		if (it->adresaInceput == p.partitieProprie->adresaInceput && it->adresaSfarsit == p.partitieProprie->adresaSfarsit)
		{
			memoriePlina.erase(it);
			break;
		}
	}
	p.partitieProprie = NULL;
	rezultateProcese.push_back(p);
	partitieNoua->dimensiune_partitie = partitieNoua->adresaSfarsit - partitieNoua->adresaInceput;
	memorieLibera.push_back(*partitieNoua);

	ReconstitueMemorieLibera();
}


void PlanificatorDeProcese()
{
	std::sort(procese.begin(), procese.end(), [](Proces& a, Proces& b) ->bool
	{
		return a.mtimp > b.mtimp;
	});
	int time = 0;

	std::cout << "Cat de utilizata a fost memoria(cate procese utilizeaza memoria la un anumit moment):\n";
	std::cout << "------------------------\n";
	while (true)
	{
		if (procese.empty() == false && time >= procese.back().mtimp)
		{
			if (RezervaMemorie(procese.back()) == true)
			{
				coadaDeProcese.push(procese.back());
				procese.pop_back();
			}
		}
		if (!coadaDeProcese.empty())
		{
			if (coadaDeProcese.front().ProcesTerminat() == true)
			{
				std::cout << memoriePlina.size() << "|"; 
				coadaDeProcese.front().timpTerminareProces = time;
				ElibereazaMemorie(coadaDeProcese.front());
				coadaDeProcese.pop();
			}

			if (time % q == 0  && coadaDeProcese.empty() == false)
			{
				Proces p = coadaDeProcese.front();
				coadaDeProcese.pop();
				coadaDeProcese.push(p);
			}

			if(!coadaDeProcese.empty())
				++(coadaDeProcese.front().timpFolosireProcesor);
		}
		++time;
		if (coadaDeProcese.empty() == true && procese.empty() == true)
			break;
	}
	std::cout << "\n------------------------\n";

}

void afisareInfoProcese()
{
	std::cout << "\nNumar procese:" << rezultateProcese.size() << std::endl << std::endl;

	int i = 0;
	for (Proces& p :rezultateProcese)
	{
		++i;
		std::cout << "Numar proces:" << i << std::endl;
		std::cout << "Timp terminare proces:" << p.timpTerminareProces << std::endl;
		std::cout << "Memorie necesara:" << p.memorieNecesara << std::endl;
		std::cout << "Durata viata proces:" << p.durataViata << std::endl;
		std::cout << "Numar cuante de timp:" << p.memorieNecesara / q << std::endl << std::endl;

		
	}
}

int main()
{
	GenerareValori();
	memorieLibera.emplace_back(0,1000); // memoria initiala = 1000 biti
	memorieLibera.front().adresaInceput = 0;
	memorieLibera.front().adresaSfarsit = 1000;
	PlanificatorDeProcese();
	afisareInfoProcese();
	system("pause");
	return 0;
}