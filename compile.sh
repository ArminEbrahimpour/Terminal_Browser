#!/bin/bash

gcc main.c -o main `xml2-config --cflags --libs` -lcurl

