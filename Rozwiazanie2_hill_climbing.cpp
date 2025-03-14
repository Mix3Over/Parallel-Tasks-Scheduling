#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <chrono> //czas
#include <csignal> //czas
#include <climits>

using namespace std;

long long result;
int maxEndTime=-1;

int maxJobs;
int MaxProcs;
int timer;
int N;
int ActiveJobsSize;

int iloscZamian = 0;

struct Job {
    int job_id;
    int arrival_time;
    int duration;
    int processors;
    float ratio;

    int StartTime;
    int EndTime;
    vector<int> ZajeteProcesory;
    bool isFinished;

    bool isProtected;
};
int findMaxEndTime(const vector<Job>& jobs) {
    auto maxJob = max_element(jobs.begin(), jobs.end(), [](const Job& a, const Job& b) {
        return a.EndTime < b.EndTime;  // Porównujemy na podstawie EndTime
    });

    if (maxJob != jobs.end()) {
        return maxJob->EndTime;  // Zwracamy największy EndTime
    } else {
        return -1;  // Gdy wektor jest pusty
    }
}

bool SortowanieArrTime(Job a, Job b){
    if(a.arrival_time != b.arrival_time){
        return a.arrival_time < b.arrival_time;
    }
    return a.duration < b.duration;
}
bool SortowanieRatio(const Job& a, const Job& b) {
    bool a_available = a.arrival_time <= timer;
    bool b_available = b.arrival_time <= timer;
    if (a_available && !b_available) {
        return true;
    }
    if (!a_available && b_available) {
        return false;
    }
    if (a_available && b_available) {
        //return a.duration*a.processors < b.duration*b.processors;
        return a.duration < b.duration;
    }
    return a.arrival_time < b.arrival_time;
}

vector<Job> oblicz_wspoczynnik(vector<Job> jobs, int MaxProcs){
    for (int i = 0; i < jobs.size(); i++) {
        float jobRatio;
        Job& job = jobs[i]; //referencja bo deklarując tworzy się kopia a my chcemy orginał
        jobRatio = job.processors;  //im większy ratio tym gorzej oceniany
        job.ratio = jobRatio;
    }
    return jobs;
}

vector<Job> wczytaj_dane(string Nazwa_Pliku, int N) {
    vector<Job> jobs;
    int nr_zadania, czas_trwania, moment_gotowosci, skip, liczba_prz_proces;
    int MaxJobs = 0;

    ifstream plik;
    plik.open(Nazwa_Pliku);

    if(!plik.is_open()) {
        cout << "NIE MOZNA OTWORZYC PLIKU" << endl;
    }
    else {
        int HowMany = 0;
        string linia;
        while (getline(plik, linia) && HowMany < N) {
            if (linia[0] == ';') {
                if (linia.find("MaxProcs:") != string::npos) {
                    vector<string> slowa_vector;
                    istringstream iss(linia); // Inicjalizacja istringstream dla wczytanej linii
                    string slowo;

                    // dodawanie slow do wektora - iss bierze i wydziela slowa z linni pomijajac biale znaki
                    while (iss >> slowo) {
                        slowa_vector.push_back(slowo);
                    }

                    //wartosc MaxProcs jest trzecim słowem w linii
                    if (slowa_vector.size() > 2) {
                        MaxProcs = stoi(slowa_vector[2]); // Konwersja trzeciego słowa na liczbę
                    }
                }
                else if (linia.find("MaxJobs:") != string::npos) {
                    vector<string> slowa_vector;
                    istringstream iss(linia);
                    string slowo;
                    while(iss>>slowo) {
                        slowa_vector.push_back(slowo);
                    }
                    //wartosc MaxJobs jest trzecim słowem w linii
                    if (slowa_vector.size() > 2) {
                        MaxJobs = stoi(slowa_vector[2]); // Konwersja trzeciego słowa na liczbę
                        maxJobs = MaxJobs;
                    }
                }
                continue;
            }
            else {
                istringstream iss(linia);
                iss >> nr_zadania >> moment_gotowosci >> skip >> czas_trwania >> liczba_prz_proces;
                if (nr_zadania != -1 && czas_trwania>0 &&  liczba_prz_proces!=-1 && moment_gotowosci != -1) {
                    HowMany++;
                    jobs.push_back({nr_zadania, moment_gotowosci, czas_trwania, liczba_prz_proces});
                }
            }
        }
        plik.close();
    }
    return jobs;
}
void zapisz_do_pliku(string NazwaPliku, vector<Job> ActiveJobs) {
    ofstream plik(NazwaPliku); // Otwarcie pliku
    if (plik.is_open()) {
        for (int i = 0; i < ActiveJobsSize; i++) {
            if (!ActiveJobs[i].ZajeteProcesory.empty()) { // Jeśli job jest aktywny

                int JobId = ActiveJobs[i].job_id;
                int StartTime = ActiveJobs[i].StartTime;
                int EndTime = ActiveJobs[i].EndTime;
                vector<int> ZajeteProcesory = ActiveJobs[i].ZajeteProcesory;

                // Konwersja procesorów do ciągu tekstowego
                string StrZajeteProcesory;
                for (int j = 0; j < ZajeteProcesory.size(); j++) {
                    StrZajeteProcesory += to_string(ZajeteProcesory[j]) + " ";
                }

                // Złożenie linii tekstu do zapisania
                stringstream linia;
                linia << JobId << " " << StartTime
                      << " " << EndTime << " " << StrZajeteProcesory;
                linia << "\n";

                //cout << "Linia do zapisu:  " << linia.str() << endl;
                // Wypisanie linii do pliku
                plik << linia.str();
            }
        }
        plik.close(); // Zamknięcie pliku
    } else {
        cout << "NIE MOZNA OTWORZYC PLIKU DO ZAPISU" << endl;
    }
}

//=================Wyjmowanie i sortowanie=======================

pair<vector<Job>, long long> WyjmowanieiSortowanie(vector<Job> container, int start, int koniec, int sposobSortowania,chrono::high_resolution_clock::time_point start_time,
    int timeLimit){
    // cout << "======nowy fragment/ sortowanie: " << sposobSortowania << " " << endl;
    // cout <<"&&&&&&&&&&&&&&&&&&&&&&&&&& poczatek: "<< start <<" &&&&&&&&& koniec: "<< koniec<< endl;
    long long wynik = result;
    int wielkosc = koniec - start;
    vector<vector<int>> tab(wielkosc, vector<int>(MaxProcs));
    vector<int> FreeSpace(wielkosc, MaxProcs);

    int StartContainerSize = container.size();
    vector<Job> FinishedJobs;
    vector<int> Konce;


    // Wrzucamy do tablicy stałe elementy (isProtected)
    for(int i = container.size() - 1; i >= 0; i--){
        if(container[i].isProtected){
            //cout<<"dodaje staly element: " << container[i].job_id <<" starttime: "<< container[i].StartTime<<" endtime: "<< container[i].EndTime <<" isProtec: "<< container[i].isProtected<< " procesors: "<< container[i].processors <<endl;
            // Przeliczanie indeksów względnie do fragmentu
            int Startwtablicy = max(container[i].StartTime, start) - start;
            int Endwtablicy = min(container[i].EndTime, koniec) - start;

            // Sprawdzenie zakresów
            if(Startwtablicy < 0) Startwtablicy = 0;
            if(Endwtablicy > (koniec - start)) Endwtablicy = koniec - start;

            for(int j = Startwtablicy; j < Endwtablicy; j++){
                for(int a = 0; a < container[i].ZajeteProcesory.size(); a++){
                    int proc = container[i].ZajeteProcesory[a];
                    if(proc >= 0 && proc < MaxProcs){
                        tab[j][proc] = container[i].job_id;
                    }
                }
                FreeSpace[j] -= container[i].processors;
            }

            Konce.push_back(Endwtablicy);
            FinishedJobs.push_back(container[i]);
            container.erase(container.begin() + i);
        } else {
            wynik -= container[i].EndTime;
        }
    }

    sort(container.begin(), container.end(), SortowanieArrTime);

    int Ttimer;
    if (!container.empty()) {  //ustawienie timera na najblizszy arrival time
        if(container[0].arrival_time - start >= 0) {
            Ttimer = container[0].arrival_time - start;

        }
        else {
            Ttimer = 0;
        }
    } else {
        //przypadek gdy container jest pusty
        wynik = -1;
        return make_pair(FinishedJobs, wynik);
    }
    int poprzedniTimer = 0;
    int CorrectedTtimer = Ttimer + start;
    while(FinishedJobs.size() != StartContainerSize){

        // Sprawdzanie limitu czasu
        auto current_time = chrono::high_resolution_clock::now();
        auto elapsed_seconds = chrono::duration_cast<chrono::seconds>(current_time - start_time).count();
        if (elapsed_seconds >= timeLimit) {
            cout << "Przekroczono limit czasu " << timeLimit << " sekund podczas WyjmowanieiSortowanie. Kończenie obliczeń." << endl;
            wynik = -1;
            return make_pair(FinishedJobs, wynik);
        }

        if(Ttimer > wielkosc){
            wynik = -1; //przypadek gdy wyszliśmy poza container (do debugogowania)
            //cout << "wynik1: "<< wynik << endl;
            return make_pair(FinishedJobs, wynik);
        }
        // Definiujemy funkcje sortujące jako wyrażenia lambda
        auto SortowanieRatio = [&CorrectedTtimer](const Job& a, const Job& b) {
            bool a_available = a.arrival_time <= CorrectedTtimer;
            bool b_available = b.arrival_time <= CorrectedTtimer;
            if (a_available && !b_available) return true;
            if (!a_available && b_available) return false;
            if (a_available && b_available) {
                return (a.duration * a.processors) < (b.duration * b.processors);
            }
            return a.arrival_time < b.arrival_time;
        };

        auto SortowanieDuration = [&CorrectedTtimer](const Job& a, const Job& b) {
            bool a_available = a.arrival_time <= CorrectedTtimer;
            bool b_available = b.arrival_time <= CorrectedTtimer;
            if (a_available && !b_available) return true;
            if (!a_available && b_available) return false;
            if (a_available && b_available) {
                return a.duration < b.duration;
            }
            return a.arrival_time < b.arrival_time;
        };

        auto SortowanieProcessors = [&CorrectedTtimer](const Job& a, const Job& b) {
            bool a_available = a.arrival_time <= CorrectedTtimer;
            bool b_available = b.arrival_time <= CorrectedTtimer;
            if (a_available && !b_available) return true;
            if (!a_available && b_available) return false;
            if (a_available && b_available) {
                return a.processors < b.processors;
            }
            return a.arrival_time < b.arrival_time;
        };

        // Sortowanie według wybranego sposobu
        if (sposobSortowania == 1) {
            sort(container.begin(), container.end(), SortowanieDuration);
        } else if (sposobSortowania == 2) {
            sort(container.begin(), container.end(), SortowanieRatio);
        } else {
            sort(container.begin(), container.end(), SortowanieProcessors);
        }
        while(!container.empty()
             && FreeSpace[Ttimer] >= container[0].processors
             && container[0].duration + Ttimer <= koniec
             && container[0].arrival_time <= CorrectedTtimer){
            int ProcesyDoZajecia = container[0].processors;

            if (Ttimer + container[0].duration > FreeSpace.size()) {
                wynik = -1;
                return make_pair(FinishedJobs, wynik);
            }

            int iloscZajetych = 0;
            for(int i=0; i<MaxProcs; i++){
                if(ProcesyDoZajecia > 0){
                    bool isFree = true;
                    for(int t = Ttimer; t < Ttimer + container[0].duration; t++){
                        if(tab[t][i] != 0){
                            isFree = false;
                            break;
                        }
                    }
                    if(isFree){
                        for(int t = Ttimer; t < Ttimer + container[0].duration; t++){
                            tab[t][i] = container[0].job_id;
                            FreeSpace[t]--;
                        }
                        container[0].ZajeteProcesory.push_back(i);
                        iloscZajetych++;
                        ProcesyDoZajecia--;
                    }
                }
            }
            if(iloscZajetych == container[0].processors) {
                // Aktualizacja wartości container[0]
                container[0].StartTime = CorrectedTtimer;
                container[0].EndTime = CorrectedTtimer + container[0].duration;
                wynik += container[0].EndTime;

                Konce.push_back(Ttimer + container[0].duration);
                FinishedJobs.push_back(container[0]);
                container.erase(container.begin());
            }
            else {
                wynik = -1; // Następuje kolizja z zadaniami
                return make_pair(FinishedJobs, wynik);
            }
             }
        //Skipowanie czasu
        // Przeskok czasu do następnego wydarzenia
        if(!Konce.empty()){
            sort(Konce.begin(), Konce.end());
            Ttimer = Konce[0];
            //cout << "skip1 do: " << Ttimer << endl;
            Konce.erase(Konce.begin(), Konce.begin()+1);
        }
        else if(!container.empty()){
            Ttimer = container[0].arrival_time - start;
            //cout << "skip2 do: " << Ttimer << endl;
            if(Ttimer < 0) Ttimer = 0;
        }
        else{
            break; // Wszystkie joby zostały zaplanowane
        }

        if (Ttimer < poprzedniTimer) {
            wynik = -1;   //do debugowania
            //cout << "wynik: "<< wynik << endl;
            return make_pair(FinishedJobs, wynik);
        }

        poprzedniTimer = Ttimer;

        CorrectedTtimer = Ttimer + start;
    }
    //cout << "KONCZE: " << "Pozostale procesy: " << container.size() << endl;
    //cout <<" zreturnowano : " <<wynik<< " sortowanie: "<< sposobSortowania << endl;
    return make_pair(FinishedJobs, wynik);
}

//=======================Rozwiazanie 2=========================
void DzielenieFragmentow(vector<Job>& ActiveJobs, long long& result, int sredniaDlugoscjoba, chrono::high_resolution_clock::time_point start_time,
    int timeLimit) {
    //int liczbaPodzialow = 100; // zmiana ilosci fragmentow
    // int dlugoscPodzialu =  931;
    int dlugoscPodzialu = sredniaDlugoscjoba;
    int liczbaPodzialow=0; //zrandomizowana dlugosc i liczba przedzialow
    int sum=0;
    for (int i=0; sum<=maxEndTime; i++) {
        liczbaPodzialow++;
        sum += dlugoscPodzialu;
    }
    int poczatek = 0;
    int koniec = poczatek + dlugoscPodzialu;
    for (int i = 0; i < liczbaPodzialow; i++) {
        vector<Job> container;
        //long long originalFragmentCj = 0; // wynik oryginalnego resulta

        auto current_time = chrono::high_resolution_clock::now();
        auto elapsed_seconds = chrono::duration_cast<chrono::seconds>(current_time - start_time).count();
        if (elapsed_seconds >= timeLimit) {
            cout << "Przekroczono limit czasu " << timeLimit << " sekund podczas DzielenieFragmentow. Kończenie obliczeń." << endl;
            return;
        }

        // Extract fragment
        for (int j = 0; j < ActiveJobs.size(); j++) {
            if (ActiveJobs[j].ZajeteProcesory.empty()) continue; // Skip empty

            if (ActiveJobs[j].StartTime >= poczatek && ActiveJobs[j].EndTime <= koniec) {
                Job NowyProces = ActiveJobs[j];
                NowyProces.ZajeteProcesory.clear();
                NowyProces.isProtected = false;
                container.push_back(NowyProces);
                //originalFragmentCj += NowyProces.EndTime; // Add to original sum
            }
            else if (ActiveJobs[j].StartTime < poczatek && ActiveJobs[j].EndTime <= koniec && ActiveJobs[j].EndTime >= poczatek) {
                ActiveJobs[j].isProtected = true;
                container.push_back(ActiveJobs[j]);
                //originalFragmentCj += ActiveJobs[j].EndTime;
            }
            else if (ActiveJobs[j].StartTime >= poczatek && ActiveJobs[j].EndTime > koniec && ActiveJobs[j].StartTime <=koniec) {
                ActiveJobs[j].isProtected = true;
                container.push_back(ActiveJobs[j]);
                //originalFragmentCj += ActiveJobs[j].EndTime;
            }
            else if (ActiveJobs[j].StartTime < poczatek && ActiveJobs[j].EndTime > koniec) {
                ActiveJobs[j].isProtected = true;
                container.push_back(ActiveJobs[j]);
                //originalFragmentCj += ActiveJobs[j].EndTime;
            }
        }

        // probowanie roznych metod sortowania
        long long bestFragmentWynik = LLONG_MAX;
        vector<Job> bestFinishedJobs;

        for (int sposobSortowania = 1; sposobSortowania <= 3; sposobSortowania++) {
            vector<Job> containerCopy = container; // Copy for each sorting method

            auto wynik_pair = WyjmowanieiSortowanie(containerCopy, poczatek, koniec, sposobSortowania, start_time, timeLimit);
            vector<Job> NoweActiveJobs = wynik_pair.first;
            long long nowyFragmentWynik = wynik_pair.second;

            //cout<< " porownoje " << bestFragmentWynik << " z " << nowyFragmentWynik  << " <- sposobSortowania: " << sposobSortowania << endl;
            if (nowyFragmentWynik != -1 && nowyFragmentWynik < bestFragmentWynik) {
                bestFragmentWynik = nowyFragmentWynik;
                bestFinishedJobs = NoweActiveJobs;
            }
            // Sprawdzanie limitu czasu po każdej iteracji sortowania
            auto current_time = chrono::high_resolution_clock::now();
            auto elapsed_seconds = chrono::duration_cast<chrono::seconds>(current_time - start_time).count();
            if (elapsed_seconds >= timeLimit) {
                cout << "Przekroczono limit czasu " << timeLimit << " sekund podczas DzielenieFragmentow (wewnętrzne sortowanie). Kończenie obliczeń." << endl;
                return;
            }
        }


        //ZMIANA WYNIKUUUUUU !!!!!!!!!!!
        if (bestFragmentWynik < result) {
            //aktualizacja wyniku
            //result = result - originalFragmentCj + bestFragmentWynik;
            result = bestFragmentWynik;
            //cout<<"!!!zmieniam wynik!!!--> " <<result<<endl;
            iloscZamian++;
            // zamiana wynikow oryginalnym vektorze active jobs
            for (auto& job : bestFinishedJobs) {
                if (job.job_id < ActiveJobs.size()) {
                    ActiveJobs[job.job_id] = job;
                }
            }
            //break;
        }

        // przeskok do nastepnego fragmentu
        poczatek = koniec;
        koniec += dlugoscPodzialu;
    }
}

//======================Rozwiazanie 1============================

long long ObliczProcesy(vector<Job>& jobs, vector<Job>& ActiveJobs) {
    vector<int> tab(MaxProcs, 0); // Wektor zamiast tablicy
    vector<int> shortestEnds; // Czas najbliższego zakończenia procesu
    int maxJobId = 0;
    for (const auto& job : jobs) {
        if (job.job_id > maxJobId) {
            maxJobId = job.job_id;
        }
    }
    cout << "START" <<endl;
    ActiveJobsSize = maxJobId + 1;
    ActiveJobs.resize(ActiveJobsSize);

    int FreeSpace = MaxProcs;
    timer = 0;
    long long SumCj_solution = 0;
    sort(jobs.begin(), jobs.end(), SortowanieRatio);

    // Główna pętla symulacji
    while ((!jobs.empty() || FreeSpace != MaxProcs)) {
        // Ustalamy, ile elementów chcemy posortować (max 400 lub mniej)
        int n = std::min(400, static_cast<int>(jobs.size()));

        // Sortujemy tylko pierwsze n elementów
        sort(jobs.begin(), jobs.begin() + n, SortowanieRatio);

        // Wyrzucanie zakończonych procesów
        for (int i = 0; i < MaxProcs; i++) {
            if (tab[i] != 0) {
                int jobId = tab[i];
                int endTime = ActiveJobs[jobId].StartTime + ActiveJobs[jobId].duration;
                if (endTime == timer) {
                    if (!ActiveJobs[jobId].isFinished) {
                        ActiveJobs[jobId].isFinished = true;
                        ActiveJobs[jobId].EndTime = timer;
                        SumCj_solution += timer; // dodanie wartości do wyniku
                        FreeSpace += ActiveJobs[jobId].processors;

                        shortestEnds.erase(remove(shortestEnds.begin(), shortestEnds.end(), endTime), shortestEnds.end());
                    }
                    tab[i] = 0;
                }
            }
        }

        // Dodawanie nowych procesów
        while (!jobs.empty() && jobs[0].processors <= FreeSpace && jobs[0].arrival_time <= timer) {
            jobs[0].StartTime = timer;

            shortestEnds.push_back(jobs[0].duration + timer);
            sort(shortestEnds.begin(), shortestEnds.end());

            int ProcesyDoZajecia = jobs[0].processors;
            int i = 0;

            // Zajmowanie wolnych procesorów
            while (ProcesyDoZajecia > 0 && i < MaxProcs) {
                if (tab[i] == 0) {
                    tab[i] = jobs[0].job_id;
                    jobs[0].ZajeteProcesory.push_back(i);
                    FreeSpace--;
                    ProcesyDoZajecia--;
                }
                i++;
            }

            // Dodajemy nowego aktywnego joba i usuwamy go z listy
            ActiveJobs[jobs[0].job_id] = jobs[0];
            jobs.erase(jobs.begin());
        }

        // Zmiana timera do najbliższego istotnego momentu
        if (!jobs.empty() && jobs[0].arrival_time > timer) {
            // Skok do czasu przyjścia nowego procesu lub zakończenia najbliższego procesu
            if (shortestEnds.empty()) {
                timer = jobs[0].arrival_time; // Skok do czasu przybycia następnego procesu
            } else {
                timer = min(jobs[0].arrival_time, shortestEnds[0]); // Skok do najbliższego istotnego wydarzenia
            }
        } else if (!shortestEnds.empty()) {
            // Jeśli nie ma nowych procesów, skaczemy do zakończenia najbliższego aktywnego procesu
            timer = shortestEnds[0];
            shortestEnds.erase(shortestEnds.begin());
        } else {
            break; // Zakończ, jeśli nie ma więcej procesów
        }
    }
    //potrzebne najwiekszy endtime do poprawnej dlugosci fragmentu w 2 rozw
    maxEndTime = findMaxEndTime(ActiveJobs);
    if (maxEndTime != -1) {
        //cout << "Najwieksza wartosc EndTime: " << maxEndTime << endl;
    } else {
        //cout << "Wektor jest pusty." << endl;
    }

    return SumCj_solution;
}

int main(int argc, char* argv[]) {
    // Sprawdzenie liczby argumentów
    if (argc != 4) {
        cout << "Użycie: " << argv[0] << " <ścieżka_do_pliku_wejsciowego> <N> <ścieżka_do_pliku_wyjsciowego>" << endl;
        return 1;
    }

    // Odczyt argumentów
    string NazwaPlikuWejsciowego = argv[1];
    string NazwaPlikuWyjsciowego = argv[3];
    try {
        N = stoi(argv[2]);
        if (N <= 0) {
            throw invalid_argument("N musi być liczbą dodatnią.");
        }
    }
    catch (const invalid_argument& e) {
        cout << "Błąd: Drugi argument (N) musi być liczbą całkowitą dodatnią." << endl;
        return 1;
    }
    catch (const out_of_range& e) {
        cout << "Błąd: Drugi argument (N) jest poza zakresem." << endl;
        return 1;
    }

    // string NazwaPlikuWejsciowego = "NASA16.swf";
    // string NazwaPlikuWyjsciowego = "ROZW1.swf";
    // N = 10000;


    timer = 0;
    vector<Job> jobs = wczytaj_dane(NazwaPlikuWejsciowego, N);
    int timeLimit = 170;

    auto start = std::chrono::high_resolution_clock::now();
    vector<Job> ActiveJobs;
    result = ObliczProcesy(jobs, ActiveJobs);

    int liczbaProcesow = ActiveJobs.size();   //potrzebne do obliczenia średniej długosci procesu
    int sumaCzasow = 0;
    for (int i = 0; i < liczbaProcesow; i++) {
        sumaCzasow += ActiveJobs[i].duration;
    }

    int sredniaDlugoscjoba = sumaCzasow / liczbaProcesow;
    if (sredniaDlugoscjoba == 0 ) {
        sredniaDlugoscjoba = 5;
    }
    // // Wywołanie funkcji DzielenieFragmentow w celu ulepszenia rozwiązania
    // DzielenieFragmentow(ActiveJobs, result, sredniaDlugoscjoba, start, timeLimit);
    //
    // // Sprawdzenie, czy przekroczono limit czasu
    // auto current = std::chrono::high_resolution_clock::now();
    // auto elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>(current - start).count();
    // if (elapsed_seconds >= timeLimit) {
    //     cout << "Przekroczono limit czasu " << timeLimit << " sekund. Kończenie obliczeń." << endl;
    //     return 0; // Wyjście z pętli
    // }


    while (true) {
        //cout << "---------ZMIANA DLUGOSCI CONTENEROW-------" <<endl;
        oblicz_wspoczynnik(jobs, MaxProcs);
        sort(jobs.begin(), jobs.end(), SortowanieArrTime);
        // Wywołanie funkcji DzielenieFragmentow w celu ulepszenia rozwiązania
        DzielenieFragmentow(ActiveJobs, result, sredniaDlugoscjoba, start, timeLimit);

        // Sprawdzenie, czy przekroczono limit czasu
        auto current = std::chrono::high_resolution_clock::now();
        auto elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>(current - start).count();
        if (elapsed_seconds >= timeLimit) {
            cout << "Przekroczono limit czasu " << timeLimit << " sekund. Kończenie obliczeń." << endl;
            break; // Wyjście z pętli
        }
        sredniaDlugoscjoba += sredniaDlugoscjoba;
        if(sredniaDlugoscjoba > maxEndTime) { // warunek zakonczenia przed ustalonym timelimit gdy już przeanalizowaliśmy wszystie podzialy
            break;
        }
    }
    // Zapis zaktualizowanego rozwiązania do pliku
    zapisz_do_pliku(NazwaPlikuWyjsciowego, ActiveJobs);
    auto end = std::chrono::high_resolution_clock::now();

    chrono::duration<double> elapsed = end - start;
    cout << "Czas wykonania: " << elapsed.count() << " sekund\n";
    cout << "Rezultat: " << result << endl;
    cout<<"END" << endl;
    return 0;
}