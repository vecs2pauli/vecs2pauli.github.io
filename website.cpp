#include <stdio.h>
#include <emscripten/emscripten.h>
#include "_extern/vecs2pauli/include/vecs2pauli.hpp"
#include <emscripten/bind.h>

using namespace emscripten;

#include <iostream>
#include <vector>
#include <utility>
#include <complex>
#include <fstream>
#include <sstream>

int main(){
    return 0;
}

extern "C" {

void printOutput(dd::PauliLIMCoset res, std::vector<std::complex<double>> vec1){
    std::cout << "Result: PauliLIMCoset = (";
    dd::printAlpha(res.alpha);
    std::cout << ", " << dd::LimEntry<>::to_string(&(res.LIM), dd::findNumQubits(vec1) - 1) << ", {";
    if (int(res.stab.size() > 1)){
        for (int i = 0; i < int(res.stab.size()-1); i++){
            std::cout << dd::LimEntry<>::to_string(&(res.stab[i]), dd::findNumQubits(vec1) - 1) << ", ";
        }
    }
    if (int(res.stab.size() > 0)){
        std::cout << dd::LimEntry<>::to_string(&(res.stab[res.stab.size()-1]), dd::findNumQubits(vec1) - 1);
    }
    std::cout << "})" << std::endl;
}

std::string makeOutputString(dd::PauliLIMCoset res, std::vector<std::complex<double>> vec1){
    std::stringstream output;
    //All transformations can be written as (3+2i)* XXX * g, where g is any product of {ZZZ, IZZ}'. 
    //If there are no solutions, write 'there are no transformations'
    
    if (res.alpha.allValues){
        output << "Any combination of a complex number and a Pauli string is a valid transformation";
        return (output.str());
    }
    else if (res.alpha.noValues){
        output << "There are no transformations";
        return (output.str());
    }
    output << "All transformations can be written as (";
    output << std::real(res.alpha.value);
    output << "+";
    output << std::imag(res.alpha.value);
    output << "i";
    output << ") * ";
    output << dd::LimEntry<>::to_string(&(res.LIM), dd::findNumQubits(vec1) - 1);
    output << " * g, where g is any product of {";
    if (int(res.stab.size() > 1)){
        for (int i = 0; i < int(res.stab.size()-1); i++){
            output << dd::LimEntry<>::to_string(&(res.stab[i]), dd::findNumQubits(vec1) - 1);
            output << ", ";
        }
    }
    if (int(res.stab.size() > 0)){
        output << dd::LimEntry<>::to_string(&(res.stab[res.stab.size()-1]), dd::findNumQubits(vec1) - 1);
    }
    output << "}";
    return (output.str());
}

std::vector<std::complex<double>> stringToVec(std::string inputstring){
    std::vector<std::complex<double>> vec;
    std::stringstream stream(inputstring);
    std::string segment;
    while(std::getline(stream, segment, ',')){
        size_t pos = 0;
        double realPart = 0;
        double imagPart = 0;
        if (segment.find("+") != std::string::npos) { // a + bi (a in R, b in R+)
            std::string token = segment.substr(0, segment.find("+"));
            realPart = std::stod(token);
            if (segment.find("i") != std::string::npos){
                token = segment.substr(segment.find("+")+1, segment.find("i") - segment.find("+"));
                if (token != "" && token != "i")
                    imagPart = std::stod(token);
                else
                    imagPart = 1;
            }
        }
        else if (segment.find("-") != std::string::npos){ 
            if (segment.find("i") != std::string::npos){ // a - bi (a in R, b in R-)
                size_t firstMinus = segment.find("-");
                std::string token;
                if (segment.find("-", firstMinus+1) != std::string::npos){ // -a - bi
                    token = segment.substr(0, segment.find("-", firstMinus+1));
                    realPart = std::stod(token);
                    if (segment.find("i") != std::string::npos){
                        token = segment.substr(segment.find("-", firstMinus+1)+1, segment.find("i") - segment.find("-", firstMinus+1));
                        if (token != "" && token != "i")
                            imagPart = (-1 * std::stod(token));
                        else
                            imagPart = -1;
                    }
                }
                else if (segment.find("i") != std::string::npos){ // (a) - bi
                    token = segment.substr(0, segment.find("-"));
                    if (token != ""){
                        realPart = std::stod(token);
                    }
                    token = segment.substr(segment.find("-")+1, segment.find("i") - segment.find("-"));
                    if (token != "")
                        imagPart = -1 * (std::stod(token));
                    else
                        imagPart = -1;
                }
            }
            else { // -a
                realPart = std::stod(segment);                
            }
        }
        else if (segment.find("i") != std::string::npos) {
            std::string token = segment.substr(0, segment.find("i"));
            if (token != "")
                imagPart = std::stod(token);
            else
                imagPart = 1;
        }
        else {
            realPart = std::stod(segment);
        }
        vec.push_back(std::complex<double>(realPart, imagPart));
    }
    return vec;
}

std::string normalFormat(uintptr_t arrayBuffer, int size)
{
    auto array = reinterpret_cast<double *>( arrayBuffer );
    std::vector<std::complex<double>> vec1, vec2;
    for (int i = 0; i < (size/2); i += 2){
        vec1.push_back(std::complex<double>(array[i], array[i+1]));
    }
    for (int i = (size/2); i < size; i += 2){
        vec2.push_back(std::complex<double>(array[i], array[i+1]));
    }
    // dd::printVec(vec1);
    // dd::printVec(vec2);
    bool foundsol = true;
    dd::PauliLIMCoset res = dd::vecs2Pauli(vec1, vec2, foundsol);
    std::string output = makeOutputString(res, vec1);
    return output;
}

std::string leftCsvFormat(uintptr_t arrayBuffer, int size, std::string leftvec){
    auto array = reinterpret_cast<double *>( arrayBuffer );
    std::vector<std::complex<double>> vec1, vec2;
    vec1 = stringToVec(leftvec);
    for (int i = 0; i < size; i += 2){
        vec2.push_back(std::complex<double>(array[i], array[i+1]));
    }
    if (vec1.size() != vec2.size()){
        return "Please input vectors of equal length\n";
    }
    // dd::printVec(vec1);
    // dd::printVec(vec2);
    bool foundsol = true;
    dd::PauliLIMCoset res = dd::vecs2Pauli(vec1, vec2, foundsol);
    std::string output = makeOutputString(res, vec1);
    return output;
}

std::string rightCsvFormat(uintptr_t arrayBuffer, int size, std::string rightvec){
    auto array = reinterpret_cast<double *>( arrayBuffer );
    std::vector<std::complex<double>> vec1, vec2;
    vec2 = stringToVec(rightvec);
    for (int i = 0; i < size; i += 2){
        vec1.push_back(std::complex<double>(array[i], array[i+1]));
    }
    if (vec1.size() != vec2.size()){
        return "Please input vectors of equal length\n";
    }
    // dd::printVec(vec1);
    // dd::printVec(vec2);
    bool foundsol = true;
    dd::PauliLIMCoset res = dd::vecs2Pauli(vec1, vec2, foundsol);
    std::string output = makeOutputString(res, vec1);
    return output;
}

std::string csvFormat(std::string leftvec, std::string rightvec){
    std::vector<std::complex<double>> vec1, vec2;
    vec1 = stringToVec(leftvec);
    vec2 = stringToVec(rightvec);
    if (vec1.size() != vec2.size()){
        return "Please input vectors of equal length\n";
    }
    // dd::printVec(vec1);
    // dd::printVec(vec2);
    bool foundsol = true;
    dd::PauliLIMCoset res = dd::vecs2Pauli(vec1, vec2, foundsol);
    std::string output = makeOutputString(res, vec1);
    return output;
}

std::string stabNormalFormat(uintptr_t arrayBuffer, int size)
{
    auto array = reinterpret_cast<double *>( arrayBuffer );
    std::vector<std::complex<double>> vec;
    for (int i = 0; i < size; i += 2){
        vec.push_back(std::complex<double>(array[i], array[i+1]));
    }
    dd::StabilizerGroupValue res = dd::findStabilizerGroup(vec);
    std::stringstream output;
    output << "The stabilizer group is generated by {";
    if (int(res.size() > 1)){
        for (int i = 0; i < int(res.size()-1); i++){
            output << dd::LimEntry<>::to_string(&(res[i]), dd::findNumQubits(vec) - 1);
            output << ", ";
        }
    }
    if (int(res.size() > 0)){
        output << dd::LimEntry<>::to_string(&(res[res.size()-1]), dd::findNumQubits(vec) - 1);
    }
    output << "}";
    return (output.str());
}

std::string stabCsvFormat(std::string vecstring){
    std::vector<std::complex<double>> vec;
    vec = stringToVec(vecstring);
    if (!dd::powerOfTwo(vec.size())){
        return "Please input vectors of length 2^n\n";
    }
    dd::StabilizerGroupValue res = dd::findStabilizerGroup(vec);
    std::stringstream output;
    output << "The stabilizer group is generated by {";
    if (int(res.size() > 1)){
        for (int i = 0; i < int(res.size()-1); i++){
            output << dd::LimEntry<>::to_string(&(res[i]), dd::findNumQubits(vec) - 1);
            output << ", ";
        }
    }
    if (int(res.size() > 0)){
        output << dd::LimEntry<>::to_string(&(res[res.size()-1]), dd::findNumQubits(vec) - 1);
    }
    output << "}";
    return (output.str());
}


}

EMSCRIPTEN_BINDINGS(my_module) {
    function("normalFormat", &normalFormat);
    function("leftCsvFormat", &leftCsvFormat);
    function("rightCsvFormat", &rightCsvFormat);
    function("csvFormat", &csvFormat);
    function("stringToVec", &stringToVec);
    function("makeOutputString", &makeOutputString);
    function("stabNormalFormat", &stabNormalFormat);
    function("stabCsvFormat", &stabCsvFormat);
}
