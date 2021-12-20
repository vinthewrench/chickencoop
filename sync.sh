#!/bin/bash
# rsync script
rsync --exclude='.DS_Store' -a /Users/vinnie/Desktop/RasberryPi/chickencoop/* vinthewrench@pi5:chickencoop
