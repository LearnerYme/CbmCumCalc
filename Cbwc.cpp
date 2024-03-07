/*

    Version: 2.0 for virgo server

    Yige Huang

    v2: 03.03.2024

    v1: 11.09.2023

    RCF Note:
        A new version of cumulant calculating.
        Yige Huang on Dec. 22, 2022
        Based on Yu Zhang's code.

*/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "TString.h"
#include "TFile.h"
#include "TH1D.h"
#include "TTree.h"
#include "TGraphErrors.h"

#include "CentralityTool/CentParams.h"
#include "CentralityTool/CentralityTool.h"

int main(int argc, char** argv){

    // do raw calculation first
    std::cout << "[LOG] Now calculting No-CBWC results.\n";
    const int MaxMult = 510;
    const int LowEventCut = 5; // to avoid error caused by low event number

    // do CBWC
    CentralityTool* mCent = new CentralityTool();
    mCent->ReadParams();
    std::cout << "[LOG] Now applying CBWC, please make sure that you have used proper CentParams.h\n";
    int centMode = atoi(argv[2]);
    if (centMode == 1) {
        std::cout << "[LOG] Centrality mode is [" << centMode << "], will use RefMult3M (only in this case)\n";
    } else {
        std::cout << "[LOG] Centrality mode is [" << centMode << "], will use RefMult3 (if you want to use RefMult3M, use [1])\n";
    }

    std::cout << "[LOG] X-axis is <Npart>, ticks are as follows: ";
    for (int i=0; i<cent_conf::nCentrality; i++) {
        std::cout << "\t" << (centMode == 1 ? cent_conf::nPart_edgeM[i] : cent_conf::nPart_edge[i]);
    }
    std::cout << std::endl;

    const int nCums = 22;
    TGraphErrors* tgs[nCums];
    TH1D* sCums[nCums];

    const char* names[nCums] = {
        "Pro_C1",
        "Pro_C2",
        "Pro_C3",
        "Pro_C4",
        "Pro_C5",
        "Pro_C6",
        "Pro_R21",
        "Pro_R32",
        "Pro_R42",
        "Pro_R51",
        "Pro_R62",
        "Pro_k1",
        "Pro_k2",
        "Pro_k3",
        "Pro_k4",
        "Pro_k5",
        "Pro_k6",
        "Pro_k21",
        "Pro_k31",
        "Pro_k41",
        "Pro_k51",
        "Pro_k61"
    };
    TFile* tfin = new TFile(Form("cum.%s.root", argv[1]));
    TFile* tfout = new TFile(Form("cum.%s.Graph.root", argv[1]), "recreate");

    for (int i=0; i<nCums; i++) {
        tgs[i] = new TGraphErrors(cent_conf::nCentrality);
        tgs[i]->SetName(names[i]);
        tfin->GetObject(names[i], sCums[i]);
    }

    const int LowMultCut = 10;
    double CentEvent[cent_conf::nCentrality] = {0.0};
    double vSum[cent_conf::nCentrality] = {0.0};
    double eSum[cent_conf::nCentrality] = {0.0};

    TH1D* hEntries;
    tfin->GetObject("ProhEntries", hEntries);
    for (int i=0; i<nCums; i++) {
        int nEvents = 0;
        // loop 1, init.
        for (int j=0; j<cent_conf::nCentrality; j++) {
            vSum[j] = 0;
            eSum[j] = 0;
            CentEvent[j] = 0;
        }

        // loop 2, sum up
        for (int j=LowMultCut; j<MaxMult; j++) {
            nEvents = hEntries->GetBinContent(j+1);
            if (nEvents < LowEventCut) { continue; }
            int curCent = mCent->GetCentrality(j+1, centMode == 1);
            if (curCent < 0) { continue; }
            if (i < 6 || (i >= 11 && i <= 16)) { // skip ratios (v calc)
                vSum[curCent] += (sCums[i]->GetBinContent(j+1) * nEvents);
            }
            eSum[curCent] += pow((sCums[i]->GetBinError(j+1) * nEvents), 2);
            CentEvent[curCent] += nEvents;
        }

        // loop 3, get weighted mean
        for (int j=0; j<cent_conf::nCentrality; j++) {
            if (i < 6 || (i >= 11 && i <= 16)) { // values
                vSum[j] /= CentEvent[j];
            } else if (i == 6) {
                vSum[j] = *(tgs[1]->GetY()+j) / *(tgs[0]->GetY()+j); // C2 / C1
            } else if (i == 7) {
                vSum[j] = *(tgs[2]->GetY()+j) / *(tgs[1]->GetY()+j); // C3 / C2
            } else if (i == 8) {
                vSum[j] = *(tgs[3]->GetY()+j) / *(tgs[1]->GetY()+j); // C4 / C2
            } else if (i == 9) {
                vSum[j] = *(tgs[4]->GetY()+j) / *(tgs[0]->GetY()+j); // C5 / C1
            } else if (i == 10) {
                vSum[j] = *(tgs[5]->GetY()+j) / *(tgs[1]->GetY()+j); // C6 / C2
            } else if (i >= 17) {
                vSum[j] = *(tgs[i-5]->GetY()+j) / *(tgs[11]->GetY()+j); // k(2~6) / k1
            }
            // for errors
            eSum[j] = sqrt(eSum[j]) / CentEvent[j];
            // set points to graphes
            tgs[i]->SetPoint(
                j, 
                centMode == 1 ? cent_conf::nPart_edgeM[j] : cent_conf::nPart_edge[j],
                vSum[j]
            );
            tgs[i]->SetPointError(j, 0.0, eSum[j]);
        }
    }

    std::cout << "[LOG] Now saving\n";
    tfout->cd();
    for (int i=0; i<nCums; i++) {
        tgs[i]->Write();
    }
    tfout->Close();
    tfin->Close();

    std::cout << "[LOG] All done!.\n";

    return 0;
}

