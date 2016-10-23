# -*- coding: utf-8 -*-
"""
Created on Sun Jan  5 10:15:34 2014

@author: Ian
"""

import yaml
import argparse as argp

if __name__ == '__main__':

    def progress(label,value=None,som=None):
        if value:
            print('{} {}%'.format(label,value))
        else:
            print(label)

    def outputError():
        raise "Invalid output fields, expected something like: -o class,Eud, but got -o {}".format(args.output)

    #get command line parameters
    parser = argp.ArgumentParser(description="Oasis montaj Python script example, called from python.gx")
    parser.add_argument("-om_state", help="YAML file contains Oasis montaj state")
    args = parser.parse_args()
    print("Oasis montaj Python example stub script.\n")

    if args.om_state == None:
        state = {}
    else:
        with open(args.om_state, 'r') as f:
            #this loads the Oasis montaj properties
            state = yaml.load(f)

    # show the properties
    print("\nCurrent Oasis montaj state\nfrom: \"{}\"\n".format(args.om_state))
    print(yaml.dump(state))

    # for example, if you want to open the current database...
    currentDB = state.get('db_current',None)
    if not currentDB:
        raise "Current database not defined.  \"db_current\" not found in properties."
    sDbName = currentDB.get('name',None)
    if not sDbName:
        raise "Database name not defined.  \"db_current/{}\" not found in properties."

    print("The currently active database is {}".format(sDbName))

    input("\nPress return to exit...")
    exit()