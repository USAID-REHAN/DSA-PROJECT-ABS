#ifndef FLIGHTMANAGER_H
#define FLIGHTMANAGER_H

#include "../CLI/Colors.h"
#include "../ds/BST.h"
#include "../ds/HashTable.h"
#include "../ds/LinkedList.h"
#include "../ds/Stack.h"
#include "../models/Aircraft.h"
#include "../models/Flight.h"
#include <iostream>
#include <string>
#include <vector>

using namespace std;

class FlightManager {
private:
  vector<Flight> *flightsRef;
  vector<Aircraft> *aircraftsRef;
  BST flightBST;
  Stack undoStack;
  LinkedList activityLog;
  HashTable<Flight> flightHash;
  HashTable<Aircraft> aircraftHash;

  // generates a unique flight id
  string generateFlightID() {
    int maxID = 0;
    for (int i = 0; i < flightsRef->size(); i++) {
      string id = (*flightsRef)[i].getFlightID();
      if (id.length() > 2) {
        int numID = stoi(id.substr(2));
        if (numID > maxID) {
          maxID = numID;
        }
      }
    }
    maxID++;
    return "HW" + to_string(maxID);
  }

  // checks for aircraft scheduling conflicts
  bool checkConflict(string aircraftID, string date, string depTime) {
    // SIMPLE CONFLICT CHECK: SAME AIRCRAFT, SAME DATE, OVERLAPPING TIME
    for (int i = 0; i < flightsRef->size(); i++) {
      Flight &f = (*flightsRef)[i];
      if (f.getAircraftID() == aircraftID && f.getDate() == date &&
          f.getDepartureTime() == depTime) {
        return true; // CONFLICT FOUND
      }
    }
    return false;
  }

  // gets current system timestamp
  string getCurrentTimestamp() {
    return "[TIME]"; // SIMPLIFIED - IN REAL APP, USE <ctime>
  }

  // builds hash tables from existing data
  void buildHashTables() {
    for (int i = 0; i < flightsRef->size(); i++) {
      Flight* f = &(*flightsRef)[i];
      flightHash.insert(f->getFlightID(), f);
    }
    for (int i = 0; i < aircraftsRef->size(); i++) {
      Aircraft* a = &(*aircraftsRef)[i];
      aircraftHash.insert(a->getAircraftID(), a);
    }
  }

public:
  // constructor for flight manager
  FlightManager(vector<Flight> *flights, vector<Aircraft> *aircrafts) {
    flightsRef = flights;
    aircraftsRef = aircrafts;
    
    buildHashTables();

    // BUILD BST FROM EXISTING FLIGHTS
    for (int i = 0; i < flightsRef->size(); i++) {
      flightBST.insert((*flightsRef)[i]);
    }
  }

  // adds a new flight to the system
  void addFlight() {
    string aircraftID, origin, dest, depTime, arrTime, date, status;
    int capacity;

    cout << Colors::BOLD << Colors::BRIGHT_CYAN
         << "\n========================================\n";
    cout << "          ADD NEW FLIGHT\n";
    cout << "========================================\n" << Colors::RESET;

    // SHOW AVAILABLE AIRCRAFT
    cout << Colors::BOLD << "AVAILABLE AIRCRAFT:\n" << Colors::RESET;
    for (int i = 0; i < aircraftsRef->size(); i++) {
      cout << "   " << (*aircraftsRef)[i].getAircraftID() << " - "
           << (*aircraftsRef)[i].getModel()
           << " (Capacity: " << (*aircraftsRef)[i].getCapacity() << ")" << endl;
    }

    cin.ignore();
    cout << Colors::BOLD << "ENTER AIRCRAFT ID: " << Colors::RESET;
    getline(cin, aircraftID);

    // FIND AIRCRAFT CAPACITY using Hash Table
    Aircraft* aircraft = aircraftHash.search(aircraftID);
    if (aircraft == nullptr) {
      cout << Colors::BOLD << Colors::BRIGHT_RED << "[-] INVALID AIRCRAFT ID!"
           << Colors::RESET << endl;
      return;
    }
    capacity = aircraft->getCapacity();

    cout << Colors::BOLD << "ENTER ORIGIN (AIRPORT CODE): " << Colors::RESET;
    getline(cin, origin);
    cout << Colors::BOLD
         << "ENTER DESTINATION (AIRPORT CODE): " << Colors::RESET;
    getline(cin, dest);
    cout << Colors::BOLD << "ENTER DEPARTURE TIME (HH:MM): " << Colors::RESET;
    getline(cin, depTime);
    cout << Colors::BOLD << "ENTER ARRIVAL TIME (HH:MM): " << Colors::RESET;
    getline(cin, arrTime);
    cout << Colors::BOLD << "ENTER DATE (DD-MM-YYYY): " << Colors::RESET;
    getline(cin, date);

    // CHECK CONFLICT
    if (checkConflict(aircraftID, date, depTime)) {
      cout
          << Colors::BOLD << Colors::BRIGHT_RED
          << "\n[-] CONFLICT DETECTED! AIRCRAFT ALREADY SCHEDULED AT THIS TIME!"
          << Colors::RESET << endl;
      return;
    }

    string newID = generateFlightID();
    Flight newFlight(newID, aircraftID, origin, dest, depTime, arrTime, date,
                     capacity, 0, "SCHEDULED");

    // PUSH TO UNDO STACK (MARK AS ADDED FOR UNDO LOGIC)
    Flight undoFlight = newFlight;
    undoFlight.setStatus("UNDO_ADD"); // INTERNAL FLAG
    undoStack.push(undoFlight);

    flightsRef->push_back(newFlight);
    
    // ADD TO HASH TABLE AND BST
    Flight* f = &flightsRef->back();
    flightHash.insert(newID, f);
    flightBST.insert(newFlight);

    activityLog.insertEnd("Added Flight: " + newID, getCurrentTimestamp());

    cout << Colors::BOLD << Colors::BRIGHT_GREEN
         << "\n[+] FLIGHT ADDED SUCCESSFULLY! FLIGHT ID: " << newID
         << Colors::RESET << endl;
  }

  // updates details of an existing flight
  void updateFlight() {
    string flightID;
    cin.ignore();
    cout << Colors::BOLD << "\nENTER FLIGHT ID TO UPDATE: " << Colors::RESET;
    getline(cin, flightID);

    // Use hash table for fast lookup
    Flight *flight = flightHash.search(flightID);
    if (flight == nullptr) {
      cout << Colors::BOLD << Colors::BRIGHT_RED << "[-] FLIGHT NOT FOUND!"
           << Colors::RESET << endl;
      return;
    }

    // SAVE TO UNDO STACK
    undoStack.push(*flight);

    cout << Colors::BOLD << Colors::BRIGHT_CYAN << "\n===== UPDATE FLIGHT "
         << flightID << " =====\n"
         << Colors::RESET;
    cout << "   [1] UPDATE DEPARTURE TIME\n";
    cout << "   [2] UPDATE ARRIVAL TIME\n";
    cout << "   [3] UPDATE DATE\n";
    cout << "   [4] UPDATE STATUS\n";
    cout << Colors::BOLD << Colors::YELLOW
         << "   > Enter choice: " << Colors::RESET;

    int choice;
    cin >> choice;
    cin.ignore();

    string newValue;
    switch (choice) {
    case 1:
      cout << Colors::BOLD
           << "ENTER NEW DEPARTURE TIME (HH:MM): " << Colors::RESET;
      getline(cin, newValue);
      flight->setDepartureTime(newValue);
      break;
    case 2:
      cout << Colors::BOLD
           << "ENTER NEW ARRIVAL TIME (HH:MM): " << Colors::RESET;
      getline(cin, newValue);
      flight->setArrivalTime(newValue);
      break;
    case 3:
      cout << Colors::BOLD << "ENTER NEW DATE (DD-MM-YYYY): " << Colors::RESET;
      getline(cin, newValue);
      flight->setDate(newValue);
      break;
    case 4:
      cout << Colors::BOLD
           << "ENTER NEW STATUS (SCHEDULED/DELAYED/CANCELLED/COMPLETED): "
           << Colors::RESET;
      getline(cin, newValue);
      flight->setStatus(newValue);
      break;
    default:
      cout << Colors::BOLD << Colors::BRIGHT_RED << "[-] INVALID CHOICE!"
           << Colors::RESET << endl;
      return;
    }

    // UPDATE IN VECTOR
    for (int i = 0; i < flightsRef->size(); i++) {
      if ((*flightsRef)[i].getFlightID() == flightID) {
        (*flightsRef)[i] = *flight;
        break;
      }
    }

    activityLog.insertEnd("Updated Flight: " + flightID, getCurrentTimestamp());
    cout << Colors::BOLD << Colors::BRIGHT_GREEN
         << "\n[+] FLIGHT UPDATED SUCCESSFULLY!" << Colors::RESET << endl;
  }

  // removes a flight from the system
  void deleteFlight() {
    string flightID;
    cin.ignore();
    cout << Colors::BOLD << "\nENTER FLIGHT ID TO DELETE: " << Colors::RESET;
    getline(cin, flightID);

    Flight *flight = flightHash.search(flightID);
    if (flight == nullptr) {
      cout << Colors::BOLD << Colors::BRIGHT_RED << "[-] FLIGHT NOT FOUND!"
           << Colors::RESET << endl;
      return;
    }

    // SAVE TO UNDO STACK
    undoStack.push(*flight);

    // DELETE FROM VECTOR
    for (int i = 0; i < flightsRef->size(); i++) {
      if ((*flightsRef)[i].getFlightID() == flightID) {
        flightsRef->erase(flightsRef->begin() + i);
        break;
      }
    }

    // DELETE FROM HASH TABLE AND BST
    flightHash.remove(flightID);
    flightBST.remove(flightID);

    activityLog.insertEnd("Deleted Flight: " + flightID, getCurrentTimestamp());
    cout << Colors::BOLD << Colors::BRIGHT_GREEN
         << "\n[+] FLIGHT DELETED SUCCESSFULLY!" << Colors::RESET << endl;
  }

  // displays all flights in the system
  void displayAllFlights() {
    if (flightsRef->empty()) {
      cout << Colors::BOLD << Colors::BRIGHT_RED
           << "\n[-] NO FLIGHTS IN SYSTEM!" << Colors::RESET << endl;
      return;
    }

    cout << Colors::BOLD << Colors::BRIGHT_CYAN
         << "\n========== ALL FLIGHTS ==========\n"
         << Colors::RESET;
    for (int i = 0; i < flightsRef->size(); i++) {
      Flight &f = (*flightsRef)[i];
      cout << Colors::BRIGHT_GREEN << "\n[FLIGHT] " << f.getFlightID()
           << Colors::RESET << " | Aircraft: " << f.getAircraftID()
           << " | Route: " << f.getOrigin() << " > " << f.getDestination()
           << "\n         Departure: " << f.getDepartureTime()
           << " | Arrival: " << f.getArrivalTime()
           << "\n         Date: " << f.getDate()
           << " | Seats: " << f.getBookedSeats() << "/" << f.getCapacity()
           << " | Status: " << f.getStatus() << endl;
      cout << "--------------------------------" << endl;
    }
    cout << Colors::BOLD << Colors::BRIGHT_CYAN
         << "=================================\n"
         << Colors::RESET;
  }

  // searches for flights by route (origin and destination)
  void searchFlightsByRoute() {
    string origin, destination;
    cin.ignore();

    cout << Colors::BOLD << Colors::BRIGHT_CYAN
         << "\n===== SEARCH FLIGHTS BY ROUTE =====\n"
         << Colors::RESET;
    cout << Colors::BOLD
         << "ENTER DEPARTURE CITY (AIRPORT CODE): " << Colors::RESET;
    getline(cin, origin);
    cout << Colors::BOLD
         << "ENTER DESTINATION CITY (AIRPORT CODE): " << Colors::RESET;
    getline(cin, destination);

    vector<Flight> matchingFlights;
    for (int i = 0; i < flightsRef->size(); i++) {
      Flight &f = (*flightsRef)[i];
      if (f.getOrigin() == origin && f.getDestination() == destination) {
        matchingFlights.push_back(f);
      }
    }

    if (matchingFlights.empty()) {
      cout << Colors::BOLD << Colors::BRIGHT_RED
           << "\n[-] NO FLIGHTS FOUND FOR ROUTE " << origin << " > "
           << destination << Colors::RESET << endl;
      return;
    }

    cout << Colors::BOLD << Colors::BRIGHT_CYAN
         << "\n========== AVAILABLE FLIGHTS: " << origin << " > " << destination
         << " ==========\n"
         << Colors::RESET;
    for (int i = 0; i < matchingFlights.size(); i++) {
      Flight &f = matchingFlights[i];
      int availableSeats = f.getCapacity() - f.getBookedSeats();
      cout << Colors::BRIGHT_GREEN << "\n"
           << (i + 1) << ". FLIGHT ID: " << f.getFlightID() << Colors::RESET
           << "\n   AIRCRAFT: " << f.getAircraftID()
           << "\n   DEPARTURE: " << f.getDepartureTime()
           << " | ARRIVAL: " << f.getArrivalTime()
           << "\n   DATE: " << f.getDate()
           << "\n   AVAILABLE SEATS: " << availableSeats << "/"
           << f.getCapacity() << "\n   STATUS: " << f.getStatus() << endl;
      cout << "   --------------------------------" << endl;
    }
    cout << Colors::BOLD << Colors::BRIGHT_CYAN
         << "====================================================\n"
         << Colors::RESET;
  }

  // searches for a flight by its id
  void searchFlightByID() {
    string flightID;
    cin.ignore();
    cout << Colors::BOLD << "\nENTER FLIGHT ID: " << Colors::RESET;
    getline(cin, flightID);

    // Use hash table for fast lookup
    Flight *flight = flightHash.search(flightID);
    if (flight == nullptr) {
      cout << Colors::BOLD << Colors::BRIGHT_RED << "[-] FLIGHT NOT FOUND!"
           << Colors::RESET << endl;
      return;
    }

    cout << Colors::BOLD << Colors::BRIGHT_CYAN
         << "\n===== FLIGHT DETAILS =====\n"
         << Colors::RESET;
    cout << Colors::BRIGHT_GREEN << "   [FLIGHT] " << flight->getFlightID()
         << Colors::RESET << "\n   Aircraft: " << flight->getAircraftID()
         << "\n   Route: " << flight->getOrigin() << " > "
         << flight->getDestination()
         << "\n   Departure: " << flight->getDepartureTime()
         << " | Arrival: " << flight->getArrivalTime()
         << "\n   Date: " << flight->getDate()
         << "\n   Seats: " << flight->getBookedSeats() << "/"
         << flight->getCapacity() << " | Status: " << flight->getStatus()
         << endl;
    cout << Colors::BOLD << Colors::BRIGHT_CYAN
         << "==========================\n"
         << Colors::RESET;
  }

  // changes the status of a specific flight
  void changeFlightStatus() {
    string flightID, newStatus;
    cin.ignore();
    cout << Colors::BOLD << "\nENTER FLIGHT ID: " << Colors::RESET;
    getline(cin, flightID);

    // Use hash table for fast lookup
    Flight *flight = flightHash.search(flightID);
    if (flight == nullptr) {
      cout << Colors::BOLD << Colors::BRIGHT_RED << "[-] FLIGHT NOT FOUND!"
           << Colors::RESET << endl;
      return;
    }

    cout << Colors::BOLD << "CURRENT STATUS: " << flight->getStatus()
         << Colors::RESET << endl;
    cout << Colors::BOLD
         << "ENTER NEW STATUS (SCHEDULED/DELAYED/CANCELLED/COMPLETED): "
         << Colors::RESET;
    getline(cin, newStatus);

    flight->setStatus(newStatus);

    // UPDATE IN VECTOR
    for (int i = 0; i < flightsRef->size(); i++) {
      if ((*flightsRef)[i].getFlightID() == flightID) {
        (*flightsRef)[i].setStatus(newStatus);
        break;
      }
    }

    activityLog.insertEnd("Changed Status of Flight " + flightID + " to " +
                              newStatus,
                          getCurrentTimestamp());
    cout << Colors::BOLD << Colors::BRIGHT_GREEN
         << "\n[+] STATUS UPDATED SUCCESSFULLY!" << Colors::RESET << endl;
  }

  // undoes the last add or update action
  void undoLastAction() {
    if (undoStack.isEmpty()) {
      cout << Colors::BOLD << Colors::BRIGHT_RED << "\n[-] NO ACTIONS TO UNDO!"
           << Colors::RESET << endl;
      return;
    }

    Flight lastFlight = undoStack.pop();
    string flightID = lastFlight.getFlightID();

    if (lastFlight.getStatus() == "UNDO_ADD") {
      // UNDO AN ADDITION = DELETE
      for (int i = 0; i < flightsRef->size(); i++) {
        if ((*flightsRef)[i].getFlightID() == flightID) {
          flightsRef->erase(flightsRef->begin() + i);
          break;
        }
      }
      // REMOVE FROM HASH TABLE AND BST
      flightHash.remove(flightID);
      flightBST.remove(flightID);
      cout << Colors::BOLD << Colors::BRIGHT_GREEN
           << "\n[+] UNDO: REMOVED ADDED FLIGHT " << flightID << Colors::RESET
           << endl;
    } else {
      // UNDO AN UPDATE OR DELETE = RESTORE
      cout << Colors::BOLD << Colors::BRIGHT_GREEN
           << "\n[+] UNDO: RESTORING FLIGHT " << flightID << Colors::RESET
           << endl;
      bool found = false;
      for (int i = 0; i < flightsRef->size(); i++) {
        if ((*flightsRef)[i].getFlightID() == flightID) {
          (*flightsRef)[i] = lastFlight;
          found = true;
          break;
        }
      }
      if (!found) {
        flightsRef->push_back(lastFlight);
        // ADD TO HASH TABLE
        Flight* f = &flightsRef->back();
        flightHash.insert(flightID, f);
      }
      // UPDATE BST (REMOVE OLD AND RE-INSERT TO RE-BALANCE OR OVERWRITE)
      flightBST.remove(flightID);
      flightBST.insert(lastFlight);
    }

    activityLog.insertEnd("Undo Action", getCurrentTimestamp());
    cout << Colors::BOLD << Colors::BRIGHT_GREEN << "[+] UNDO SUCCESSFUL!"
         << Colors::RESET << endl;
  }

  // displays the system activity log
  void viewActivityLog() { activityLog.displayAll(); }
};

#endif