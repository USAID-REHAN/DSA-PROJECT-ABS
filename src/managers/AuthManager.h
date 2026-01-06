#ifndef AUTHMANAGER_H
#define AUTHMANAGER_H

#include "../CLI/Colors.h"
#include "../ds/HashTable.h"  // ADD THIS
#include "../models/Passenger.h"
#include "FileManager.h"
#include <iostream>
#include <string>
#include <vector>

using namespace std;

class AuthManager {
private:
  vector<Passenger> *passengersRef;
  HashTable<Passenger> passengerHashByID;    // ADD THIS
  HashTable<Passenger> passengerHashByEmail; // ADD THIS

  string generatePassengerID() {
    int maxID = 0;
    for (int i = 0; i < passengersRef->size(); i++) {
      string id = (*passengersRef)[i].getPassengerID();
      if (id.length() > 1) {
        int numID = stoi(id.substr(1));
        if (numID > maxID) {
          maxID = numID;
        }
      }
    }
    maxID++;
    return "P" + to_string(maxID);
  }

  // ADD THIS - Build hash tables from existing data
  void buildHashTables() {
    for (int i = 0; i < passengersRef->size(); i++) {
      Passenger* p = &(*passengersRef)[i];
      passengerHashByID.insert(p->getPassengerID(), p);
      passengerHashByEmail.insert(p->getEmail(), p);
    }
  }

public:
  AuthManager(vector<Passenger> *passengers) { 
    passengersRef = passengers;
    buildHashTables();  // ADD THIS
  }

  // MODIFY THIS - Use hash table for faster lookup
  Passenger *loginPassenger(string email, string password) {
    Passenger* p = passengerHashByEmail.search(email);  // HASH TABLE LOOKUP
    if (p != nullptr && p->getPassword() == password) {
      return p;
    }
    return nullptr;
  }

  bool signupPassenger(string name, string email, string password, string phone,
                       string type) {

    // CHECK IF EMAIL ALREADY EXISTS - Using Hash Table
    if (passengerHashByEmail.search(email) != nullptr) {  // HASH TABLE LOOKUP
      return false;
    }

    string newID = generatePassengerID();
    Passenger newPassenger(newID, name, email, password, phone, type);
    passengersRef->push_back(newPassenger);
    
    // ADD TO HASH TABLES
    Passenger* p = &passengersRef->back();
    passengerHashByID.insert(newID, p);
    passengerHashByEmail.insert(email, p);
    
    return true;
  }

  // ADD THIS - Fast passenger lookup by ID
  Passenger* findPassengerByID(string passengerID) {
    return passengerHashByID.search(passengerID);
  }

  bool loginAdmin(string filename, string username, string password) {
    return FileManager::verifyAdmin(filename, username, password);
  }
};

#endif