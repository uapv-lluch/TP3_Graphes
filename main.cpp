#include <iostream>
#include <vector>
#include <fstream>
#include <list>
#include <cmath>
#include <iomanip>

using namespace std;


struct Arc {
    int poids;
    int sommetDepart;
    int sommetArrivee;

    bool operator <(const Arc & arc) const {
        return poids < arc.poids;
    }
    bool operator==(const Arc & arc) const {
        return (sommetDepart == arc.sommetDepart && sommetArrivee == arc.sommetArrivee && poids == arc.poids);
    }
};

struct Sommet {
    int id;
    vector<Arc> arcsEntrants;
    vector<Arc> arcsSortants;
    int debutAuPlusTot;
    int debutAuPlusTard;

    bool operator==(const Sommet & sommet) const {
        return id == sommet.id;
    }
};

struct Graphe {
    vector<Arc> arcs;
    vector<Sommet> sommets;
};

vector<string> split(const string& str, char delimiter) {
    vector <string> result;
    size_t begin = 0;
    size_t index = str.find(delimiter);
    while (index != string::npos) {
        string sub = str.substr(begin, index-begin);
        result.push_back(sub);
        begin = index+1;
        index = str.find(delimiter, begin);
    }
    string sub = str.substr(begin, str.find('\n')-begin);
    result.push_back(sub);
    return result;
}

void displaySommets(const Graphe& graphe) {
    cout << setw(2) << "Id" << setw(6) << "Tot" << setw(6) << "Tard" << endl;
    for (Sommet sommet : graphe.sommets) {
        cout << setw(2) << sommet.id << setw(6) << sommet.debutAuPlusTot << setw(6) << sommet.debutAuPlusTard << endl;
    }
}

Graphe graphePotentielTaches(ifstream & file) {
    string line;
    int n = 0;
    int nbSommets = 0;
    vector<Sommet> sommets; // { id, debutAuPlusTot, debutAuPlusTard }
    vector<Arc> arcs; // { poids , sommetDepart, sommetArrivee }
    vector<int> duree;
    while (getline(file, line)) {
        // On lit les données et on stocke le nombre de sommets et les coordonnées
        if (n == 0) { // Nombre de sommets
            nbSommets = stoi(line)+1;
            duree.resize(nbSommets);
        } else { // Ordre
            vector<string> splittedLine = split(line, ' ');
            Sommet sommetArrivee = {stoi(splittedLine[0])-1};
            duree[sommetArrivee.id] = stoi(splittedLine[1]);
            if (splittedLine.size() > 2) {
                Arc arc;
                for (int i = 2 ; i < splittedLine.size() ; ++i) {
                    Sommet sommetDepart = {stoi(splittedLine[i])-1};
                    arc = {duree[sommetDepart.id], sommetDepart.id, sommetArrivee.id};
                    arcs.push_back(arc);
                    sommetArrivee.arcsEntrants.push_back(arc);
                    sommets[sommetDepart.id].arcsSortants.push_back(arc);
                }
            }
            sommets.push_back(sommetArrivee);
        }
        ++n;
    }
    Sommet sommetFinal = {n-1};
    for (const Sommet& sommet : sommets) {
        if (sommet.arcsSortants.empty()) {
            Arc arcFinal = {duree[sommet.id], sommet.id, sommetFinal.id};
            sommetFinal.arcsEntrants.push_back(arcFinal);
            arcs.push_back(arcFinal);
        }
    }
    sommets.push_back(sommetFinal);
    return {arcs, sommets};
}

vector<int> bellmanLong(vector<Arc> arcs, int nbSommets, int s) {
//    int s = 0;
    vector<int> dist(nbSommets);
    vector<int> pred(nbSommets);
    dist[s] = 0;
    for (int i = 0 ; i < nbSommets ; ++i) {
        if (i != s) {
            dist[i] = -1;
        }
    }
    vector<Arc> sousArcs;
    for (int i = 0; i < arcs.size() ; ++i) {
        if (arcs[i].sommetDepart >= s) {
            sousArcs.push_back(arcs[i]);
        }
    }
    bool stop = false;
    while (!stop) {
        stop = true;
        for (auto it : sousArcs) {
            if (dist[it.sommetArrivee] < dist[it.sommetDepart] + it.poids) {
                dist[it.sommetArrivee] = dist[it.sommetDepart] + it.poids;
                pred[it.sommetArrivee] = it.sommetDepart;
                stop = false;
            }
        }
    }
    return dist;
}

Graphe earlierStartingDate(Graphe & graphe) {
    vector<int> dist = bellmanLong(graphe.arcs, graphe.sommets.size(), 0);
    for (Sommet & sommet : graphe.sommets) {
        sommet.debutAuPlusTot = dist[sommet.id];
    }
    return graphe;
}

Graphe latestStartingDate(Graphe & graphe) {
    int lastSommetId = graphe.sommets.size()-1;
    vector<int> distAuDebut = bellmanLong(graphe.arcs, graphe.sommets.size(), 0);
    for (Sommet & sommet : graphe.sommets) {
        vector<int> dist = bellmanLong(graphe.arcs, graphe.sommets.size(), sommet.id);
        sommet.debutAuPlusTard = distAuDebut[lastSommetId] - dist[lastSommetId];
    }
    return graphe;
}

vector<Sommet> getCriticalPath(Graphe graphe) {
    vector<Sommet> criticalPath;
    Sommet premierSommet = graphe.sommets[0];
    criticalPath.push_back(premierSommet);
    bool stop = false;
    while (!stop) {
        stop = true;
        Sommet sommetCourant = criticalPath[criticalPath.size()-1];
        for (Arc arc : sommetCourant.arcsSortants) {
            Sommet sommetSuivant = graphe.sommets[arc.sommetArrivee];
            if (sommetSuivant.debutAuPlusTard - sommetSuivant.debutAuPlusTot == 0) {
                criticalPath.push_back(sommetSuivant);
                stop = false;
                break;
            }
        }
    }
    criticalPath.push_back(graphe.sommets[graphe.sommets.size()-1]);
    return criticalPath;
}

void displayCriticalPath(vector<Sommet> criticalPath) {
    cout << "Chemin critique : ";
    for (Sommet sommet : criticalPath) {
        cout << setw(2) << sommet.id;
    }
    cout << endl;
}


int main() {
    ifstream file("../resources/input.txt", ios::in);
    if (file) {
        Graphe graphe; // { arcs, sommets }
        graphe = graphePotentielTaches(file);
        earlierStartingDate(graphe);
        latestStartingDate(graphe);
        displaySommets(graphe);
        vector<Sommet> criticalPath = getCriticalPath(graphe);
        displayCriticalPath(criticalPath);
    } else {
        cerr << "Error file";
        return -1;
    }
    return 0;
}