#ifndef BOOKINGMANAGER_H
#define BOOKINGMANAGER_H

#include "../CLI/Colors.h"
#include "../ds/HashTable.h"
#include "../ds/PriorityQueue.h"
#include "../ds/Queue.h"
#include "../models/Booking.h"
#include "../models/Flight.h"
#include "../models/Passenger.h"
#include <iostream>
#include <string>
#include <vector>

using namespace std;

class BookingManager {
private:
  vector<Booking> *bookingsRef;
  vector<Flight> *flightsRef;
  vector<Passenger> *passengersRef;
  Queue waitlistQueue;
  PriorityQueue boardingQueue;
  HashTable<Booking> bookingHash;

  // generates a unique booking id
  string generateBookingID() {
    int maxID = 0;
    for (int i = 0; i < bookingsRef->size(); i++) {
      string id = (*bookingsRef)[i].getBookingID();
      if (id.length() > 1) {
        int numID = stoi(id.substr(1));
        if (numID > maxID) {
          maxID = numID;
        }
      }
    }
    maxID++;
    return "B" + to_string(maxID);
  }

  // generates a seat number based on index
  string generateSeatNumber(int seatNum) {
    char row = 'A' + (seatNum / 6);
    int col = (seatNum % 6) + 1;
    return string(1, row) + to_string(col);
  }

  // helper to find a flight by its id
  Flight *findFlight(string flightID) {
    for (int i = 0; i < flightsRef->size(); i++) {
      if ((*flightsRef)[i].getFlightID() == flightID) {
        return &(*flightsRef)[i];
      }
    }
    return nullptr;
  }

  // helper to find a passenger by their id
  Passenger *findPassenger(string passengerID) {
    for (int i = 0; i < passengersRef->size(); i++) {
      if ((*passengersRef)[i].getPassengerID() == passengerID) {
        return &(*passengersRef)[i];
      }
    }
    return nullptr;
  }

  // builds hash table from existing bookings
  void buildBookingHash() {
    for (int i = 0; i < bookingsRef->size(); i++) {
      Booking* b = &(*bookingsRef)[i];
      bookingHash.insert(b->getBookingID(), b);
    }
  }

public:
  // constructor for booking manager
  BookingManager(vector<Booking> *bookings, vector<Flight> *flights,
                 vector<Passenger> *passengers) {
    bookingsRef = bookings;
    flightsRef = flights;
    passengersRef = passengers;
    buildBookingHash();
  }

  // handles the flight booking process for a passenger
  void bookFlight(string passengerID) {
    string flightID;
    cin.ignore();
    cout << Colors::BOLD << "\nENTER FLIGHT ID TO BOOK: " << Colors::RESET;
    getline(cin, flightID);

    Flight *flight = findFlight(flightID);
    if (flight == nullptr) {
      cout << Colors::BOLD << Colors::BRIGHT_RED << "[-] FLIGHT NOT FOUND!"
           << Colors::RESET << endl;
      return;
    }

    // check if already booked
    for (int i = 0; i < bookingsRef->size(); i++) {
      if ((*bookingsRef)[i].getPassengerID() == passengerID &&
          (*bookingsRef)[i].getFlightID() == flightID &&
          (*bookingsRef)[i].getBookingStatus() != "CANCELLED") {
        cout << Colors::BOLD << Colors::BRIGHT_RED
             << "\n[-] YOU HAVE ALREADY BOOKED THIS FLIGHT!" << Colors::RESET
             << endl;
        return;
      }
    }

    string bookingID = generateBookingID();

    if (flight->isFull()) {
      // add to waitlist
      waitlistQueue.enqueue(passengerID, flightID);
      Booking newBooking(bookingID, passengerID, flightID, "WAITLIST",
                         "WAITLIST");
      bookingsRef->push_back(newBooking);
      
      // ADD TO HASH TABLE
      Booking* b = &bookingsRef->back();
      bookingHash.insert(bookingID, b);
      
      cout << Colors::BOLD << Colors::BRIGHT_YELLOW
           << "\n[!] FLIGHT IS FULL! ADDED TO WAITLIST." << Colors::RESET
           << endl;
      cout << "BOOKING ID: " << bookingID << endl;
    } else {
      // confirm booking
      string seatNumber = generateSeatNumber(flight->getBookedSeats());
      Booking newBooking(bookingID, passengerID, flightID, seatNumber,
                         "CONFIRMED");
      bookingsRef->push_back(newBooking);
      
      // ADD TO HASH TABLE
      Booking* b = &bookingsRef->back();
      bookingHash.insert(bookingID, b);

      flight->setBookedSeats(flight->getBookedSeats() + 1);

      // update flight in vector
      for (int i = 0; i < flightsRef->size(); i++) {
        if ((*flightsRef)[i].getFlightID() == flightID) {
          (*flightsRef)[i].setBookedSeats(flight->getBookedSeats());
          break;
        }
      }

      // add to boarding priority queue
      Passenger *passenger = findPassenger(passengerID);
      if (passenger != nullptr) {
        boardingQueue.insert(*passenger);
      }

      cout << Colors::BOLD << Colors::BRIGHT_GREEN << "\n[+] BOOKING CONFIRMED!"
           << Colors::RESET << endl;
      cout << "BOOKING ID: " << bookingID << endl;
      cout << "SEAT NUMBER: " << seatNumber << endl;
    }
  }

  // cancels a booking for a passenger using hash table for fast lookup
  void cancelBooking(string passengerID) {
    string bookingID;
    cin.ignore();
    cout << Colors::BOLD << "\nENTER BOOKING ID TO CANCEL: " << Colors::RESET;
    getline(cin, bookingID);

    // USE HASH TABLE for O(1) lookup
    Booking *booking = bookingHash.search(bookingID);

    if (booking == nullptr || booking->getPassengerID() != passengerID) {
      cout << Colors::BOLD << Colors::BRIGHT_RED << "[-] BOOKING NOT FOUND!"
           << Colors::RESET << endl;
      return;
    }

    if (booking->getBookingStatus() == "CANCELLED") {
      cout << Colors::BOLD << Colors::BRIGHT_RED
           << "[-] BOOKING ALREADY CANCELLED!" << Colors::RESET << endl;
      return;
    }

    string flightID = booking->getFlightID();
    string oldStatus = booking->getBookingStatus();
    booking->setBookingStatus("CANCELLED");

    Flight *flight = findFlight(flightID);
    if (flight != nullptr && oldStatus != "WAITLIST") {
      flight->setBookedSeats(flight->getBookedSeats() - 1);

      // update flight in vector
      for (int i = 0; i < flightsRef->size(); i++) {
        if ((*flightsRef)[i].getFlightID() == flightID) {
          (*flightsRef)[i].setBookedSeats(flight->getBookedSeats());
          break;
        }
      }

      // check waitlist and promote
      QueueNode *waitlistPassenger = waitlistQueue.dequeueForFlight(flightID);
      if (waitlistPassenger != nullptr) {
        // find waitlist booking and confirm it
        for (int i = 0; i < bookingsRef->size(); i++) {
          if ((*bookingsRef)[i].getPassengerID() ==
                  waitlistPassenger->passengerID &&
              (*bookingsRef)[i].getFlightID() == flightID &&
              (*bookingsRef)[i].getBookingStatus() == "WAITLIST") {

            string seatNumber = generateSeatNumber(flight->getBookedSeats());
            (*bookingsRef)[i].setSeatNumber(seatNumber);
            (*bookingsRef)[i].setBookingStatus("CONFIRMED");

            flight->setBookedSeats(flight->getBookedSeats() + 1);

            // update flight in vector
            for (int j = 0; j < flightsRef->size(); j++) {
              if ((*flightsRef)[j].getFlightID() == flightID) {
                (*flightsRef)[j].setBookedSeats(flight->getBookedSeats());
                break;
              }
            }

            cout << Colors::BOLD << Colors::BRIGHT_GREEN
                 << "\n[+] WAITLIST PASSENGER "
                 << waitlistPassenger->passengerID << " PROMOTED TO CONFIRMED!"
                 << Colors::RESET << endl;
            break;
          }
        }
        delete waitlistPassenger;
      }
    }

    cout << Colors::BOLD << Colors::BRIGHT_GREEN
         << "\n[+] BOOKING CANCELLED SUCCESSFULLY!" << Colors::RESET << endl;
  }

  // displays all bookings for a specific passenger
  void viewPassengerBookings(string passengerID) {
    bool found = false;

    cout << Colors::BOLD << Colors::BRIGHT_CYAN
         << "\n========== YOUR BOOKINGS ==========\n"
         << Colors::RESET;
    for (int i = 0; i < bookingsRef->size(); i++) {
      if ((*bookingsRef)[i].getPassengerID() == passengerID) {
        Booking &b = (*bookingsRef)[i];
        cout << Colors::BRIGHT_GREEN << "\n[BOOKING] " << b.getBookingID()
             << Colors::RESET << " | Flight: " << b.getFlightID()
             << " | Seat: " << b.getSeatNumber()
             << " | Status: " << b.getBookingStatus() << endl;
        cout << "------------------------" << endl;
        found = true;
      }
    }

    if (!found) {
      cout << Colors::BOLD << Colors::BRIGHT_RED << "   [-] NO BOOKINGS FOUND!"
           << Colors::RESET << endl;
    }
    cout << Colors::BOLD << Colors::BRIGHT_CYAN
         << "==================================\n"
         << Colors::RESET;
  }

  // displays the current waitlist for a flight
  void viewWaitlist() {
    string flightID;
    cin.ignore();
    cout << Colors::BOLD << "\nENTER FLIGHT ID: " << Colors::RESET;
    getline(cin, flightID);

    waitlistQueue.display(flightID);
  }

  // displays the boarding order based on priority
  void viewBoardingPriority() { boardingQueue.displayBoardingOrder(); }
};

#endif