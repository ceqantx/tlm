#include <iostream>
using namespace std;
#include <fstream>
#include <string>
#include <iomanip>
#include <vector>
#include <numeric>
#include "interpolation.h"
#include "stdafx.h"

vector<double> linreg(const vector<double>& x, const vector<double>& y);

//include the option to change the voltage range for the linear fit

int main(int argc, char **argv) {
    ifstream infile;
    ofstream outfile;
    string comp;
    string vrange;
    double fitrangelow, fitrangehigh;
    int defargc = 2;
    char *defargv[] = {"500C.csv", "0.01", "30", "10","15"};


    if (argc == 0) { //if no filename is passed
        cout << "Please specify a file path." << endl;
        return 0;
    } else if (argc == 2) { //if no arguments are passed
        argc = defargc;
        fitrangelow = stod(defargv[3]);
        fitrangehigh = stod(defargv[4]);
        comp = defargv[1];
        vrange = defargv[2];
    } else if (argc == 3) {
        argc = 3;
        comp = argv[2];
        vrange = defargv[2];
    } else if (argc == 4) {
        argc = 4;

            comp = argv[2];
            vrange = argv[3];
    }
    else if(argc==5){
        argc = 5;
        cout << argv[2] << endl;
        if( strcmp(argv[2],"-f") || strcmp(argv[2],"-F")){


            fitrangelow = stod(argv[3]);
            fitrangehigh = stod(argv[4]);
            comp = defargv[1];
            vrange = defargv[2];

        }
        else{
            cout << "Incorrect arguments!" << endl;
            return 1;
        }
    }

    string infilename = argv[1];

//    EXTRACT PLOT NAME FROM FILE NAME
    string graphname = infilename;
    graphname.erase(graphname.size() - 4, graphname.size());
    string outfilename = "rvd";

//    DEFINE VARIABLES FOR DATA EXTRACTION
    string dummy;
    vector<vector<double>> V, I;
    vector<double> v1, i2;
    istringstream parse;

//    OPEN FILE

    infile.open(infilename);
    if (!infile.is_open()) {
        cout << "error opening file" << endl << "please check file name" << endl;
        return 0;
    }

    getline(infile, dummy);

    parse.str(dummy);


// READ FILES AND PARSE DATA INTO A MATRIX

    for (int i = 0; getline(infile, dummy); i++) {
        istringstream parse2;
        parse2.str(dummy);
        string value;
        for (int j = 0; getline(parse2, value, ','); j++) {
            // only add to number of measurements if this is the first time the loop runs.
            if (!i) {
                V.push_back(v1);
                I.push_back(i2);
            }

            if(!value.empty()){
                I[j].push_back(stod(value));}

                getline(parse2, value, ',');
            if(!value.empty()){
                V[j].push_back(stod(value));
                cout << setw(20) << V[j][i] << '\t' << setw(20) << I[j][i] << '\t';
            }
        }
        cout << endl;
    }
    cout << V[1].size() << endl;
    cout << V.size() << endl;
    infile.close();


// Linear fit and statistics
    vector<vector<double>> fitV,fitI;
    // Make a matrix for the voltage and current values to be used for a linear fit.
    // Linear fit for a range from fitrangelow <= V <= fitrangehigh
    for (int i = 0; i < V.size(); i++) {

        fitV.push_back(v1);
        fitI.push_back(i2);

        for (int j = 0; j < V[i].size(); j++) {
            if (I[i][j] >= fitrangelow && I[i][j] <= fitrangehigh) {
                cout << V[i][j] << "   " << I[i][j] << endl;
                fitV[i].push_back(V[i][j]);
                fitI[i].push_back(I[i][j]);

            }

        }
        cout << endl;

    }
    cout << endl;


    // this would not work for the Keysight data since it only analyzes one set of data
    vector<vector<double>> vifitout;
    vector<double> R,vrbd,rdfitout;
    for(int i = 0; i < fitV.size(); i++){
        vifitout.push_back(v1);
        vifitout[i] = linreg(fitI[i],fitV[i]);
        R.push_back(vifitout[i][0]);
        vrbd.push_back(vifitout[i][1]);
        cout << vifitout[i][0] << "   " << vifitout[i][1] << endl;
    }
// linear fit the resistance vs pad distance

    vector<double> pdist;
    double rc,rs,rhoc,w=150;
    // write the resistance vs pad distance to file
    ofstream outfile3;
    outfile3.open(graphname+"-r-vs-d-"+to_string(static_cast<int>(fitrangelow))+"-to-"+to_string(static_cast<int>(fitrangehigh))+ ".txt");
    outfile3 << "d,r,vrbd" << endl;
    for(int i =0; i < R.size(); i++){

        pdist.push_back(((i%6)+2)*5);
        outfile3 << pdist[i] << "," << R[i] << "," << vrbd[i]<< endl;

    }

    rdfitout = linreg(pdist,R);
    rs = rdfitout[0]*w;
    rc = rdfitout[1]/2;
    rhoc = (rc*rc*w*w)/rs/10000/10000;
    outfile3 << "least squares fit from V=" << fitrangelow << " to V=" << fitrangehigh << endl;
    outfile3 << "slope," << rdfitout[0] << endl;
    outfile3 << "int," << rdfitout[1] << endl;
    outfile3 << "rs,"<< rs << endl;
    outfile3 << "rc," << rc << endl;
    outfile3 << "rhoc," << rhoc << endl;



//    // PLOTTING THE I-V
//    ofstream outfile2;
//    int index;
//    for (int i = 0, p = 1; i < V.size(); i++) {
//        string n;
//        n = ".txt";
//        if (!(i % 6)) {
//            if (i > 0) { system("gnuplot ./command.txt"); }
//            outfile2.close();
//            outfile2.open("command.txt", ios::out | ios::trunc);
//            outfile2 << "set term png enhanced" << endl
//                     << "set encoding utf8" << endl
//                     << "set grid lw 1"  << endl
//                     << "set border lw 3" << endl
//                     << "set output \"" + graphname + ".png\" " << endl;
//            outfile2 << "set title \"" + graphname + "\"" << endl
//                     << "set label  \"R_s  = " << scientific << setprecision(1) << rs << " Ω/sq.\" at -" << stod(vrange)-1<< ",0.008 left" << endl
//                     << "set label \"\ρ_c = " << scientific << setprecision(1) << rhoc << " Ωcm^2\" at -" << stod(vrange)-1 << ",0.0065 left" << endl
//                     << "set xlabel \"Voltage (V)\"" << endl;
//            outfile2 << "set ylabel \"Current (A)\"" << endl << "set xrange [-" + vrange + ":" + vrange + "]"
//                     << endl;
//            outfile2 << "set yrange [-" + comp + ":" + comp + "]" << endl;
//            outfile2 << "set key right bottom" << endl;
//            outfile2 << "plot ";
//            p++;
//        }
//        index = (i % 6) + 1; // this makes no sense!!!!!!!
//
//        outfile2 << "\"" + to_string(i) + n + "\"" << " with line lw 3"
//                 << "title \"" + to_string(5 * index) + "um\", \\" << endl;
//        outfile.open(to_string(i) + n);
//        //Write data to files
//        for (int j = 0; j < V[i].size(); j++) {
//            outfile << V[i][j] << " " << I[i][j] << endl;
//        }
//
//        outfile.close();
//    }
//    system("gnuplot ./command.txt");
//    outfile2.clear();
//    outfile2.close();


//  REMOVE FILES
    remove("command.txt");
    for (int i = 0; i < V.size(); i++) {

        remove((to_string(i) + ".txt").c_str());


    }

}


vector<double> linreg(const vector<double>& x, const vector<double>& y) {


    if(x.size()!=y.size()){cout << "vectors must be equal in size! " << endl; exit(1);}

    int n = x.size();
    double sumX = accumulate(x.begin(), x.end(), 0.0);
    double sumY = accumulate(y.begin(), y.end(), 0.0);
    double sumX2 = inner_product(begin(x), end(x), begin(x), 0.0);
    double sumXY = inner_product(begin(x), end(x), begin(y),0.0);

    double xmean = sumX/n;
    double ymean = sumY/n;
    double slope = (sumXY-sumX*ymean) / (sumX2-sumX*xmean);
    double intercept = ymean - slope*xmean;

    vector<double> output;
    output.push_back(slope);
    output.push_back(intercept);

    return output;


}