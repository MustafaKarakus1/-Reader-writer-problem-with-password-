READER-WRITER-PROBLEM-WITH-PASSWORD
Security manager access with seamless concurrency control.

Tags: concurrent reader-writer synchronization
Built with the tools and technologies: ![badges]

Table of Contents
Overview

Getting Started

Prerequisites

Installation

Usage

Testing

Overview
The Reader-Writer Problem with Password tool is a powerful solution designed to manage concurrent access in multi-threaded applications, ensuring data integrity and thread safety.

Why Reader-Writer Problem with Password?
This project showcases the complexities of concurrent programming by implementing a robust reader-writer model that has features including:

Concurrent Access Management: Safely facilitates access to shared resources, preventing data corruption.

Thread Safety with Synchronization: Utilizes mutexes and semaphores to eliminate conflicts between reader and writer threads.

Comprehensive Logging: Outputs detailed logs of thread activities for effective monitoring and debugging.

Accurate Validation: Ensures that only authorized threads can perform specific actions, enhancing security.

Real and Dummy Implementations: Offers both real and dummy readers/writers for flexible testing and development scenarios.

Getting Started
Prerequisites
This project requires the following dependencies:

Programming Language: C

Package Manager: Make

Installation
Clone the repository:


$ git clone https://github.com/MustafaKarakus1/-Reader-writer-problem-with-password-.git
Navigate to the project directory:


$ cd Reader-writer-problem-with-password
Install the dependencies:

Using make:

$ make deps
Usage
Run the project with:

Using make:

$ make run
Testing
reader-writer-problem-with-password uses the test_framework test framework. Run the test suite with:

Using make:

$ make test
