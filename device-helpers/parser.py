import yaml
import sys

#SIMULATION_PORT = 3000
SIMULATION_PORT = 3000
#FIRST_DGI_PORT = 5001
FIRST_DGI_PORT = 23011
SIMULATION_HOST = "192.168.200.77"

def device_to_type(device):
    if device.find("SST") != -1:
        return "Sst"
    elif device.find("FID") != -1:
        return "Fid"
    elif device.find("FREQ") != -1:
        return "Omega"
    elif device.find("DRER") != -1:
        return "Drer"
    elif device.find("DESD") != -1:
        return "Desd"
    elif device.find("LOAD") != -1:
        return "Load"
    elif device.find("LOGGER") != -1:
        return "Logger"
    return "UNDEFINED"

def da_device_chunk(config, devices, etype):
    index = 1
    chunk = "<{}>\n".format(etype)
    for device in devices:
        for d in config['simulation'][etype]:
            if d['device'] == device:
                chunk += "<entry index='{}'>\n".format(index)
                chunk += "  <device>{}</device>\n".format(device)
                for sig in d['signals']:
                    chunk += "  <signal>{}</signal>\n".format(sig)
                chunk += "  <type>{}</type>\n".format(device_to_type(device))
                chunk += "</entry>\n"
                index += 1
    chunk += "</{}>\n".format(etype)
    return chunk

def sim_device_chunk(config, fp, etype):
    i = 1
    fp.write( "<{}>\n".format(etype))
    for d in config['simulation'][etype]:
        fp.write( "<entry index='{}'>\n".format(i))
        fp.write( "  <device>{}</device>\n".format(d['device']))
        for sig in d['signals']:
            fp.write( "  <signal>{}</signal>\n".format(sig))
        fp.write( "</entry>\n" )
        i += 1
    fp.write( "</{}>\n".format(etype))

def main():
    yam = open(sys.argv[1])
    config = yaml.load(yam)
    print config
    adapter = open('simulation.xml','w')
    adapter.write( "<root>\n" )
    adapter.write( "<adapter type='simulation' port='{}'> \n".format(SIMULATION_PORT))
   
    sim_device_chunk(config, adapter, "state")
 
    sim_device_chunk(config, adapter, "command")   
    
    adapter.write("</adapter>\n")
 
    i = 0
    for lst in config['DGIs']:
        # Write the DGI Chunk
        fp = open("dgi{}.xml".format(i+1),'w')
        fp.write("<root>\n")
        fp.write("<adapter name='PSCADSimulation' type='rtds'>\n")
        fp.write("<info>\n")
        fp.write("<host>{}</host>\n".format(SIMULATION_HOST))
        fp.write("<port>{}</port>\n".format(FIRST_DGI_PORT+i))
        fp.write("</info>\n")
    
        adapter.write( "<adapter type='rtds' port='{}'> \n".format(FIRST_DGI_PORT+i))

        # Start the simulation chunk
        chunk = da_device_chunk(config, lst, "state")

        #Put the chunk in the files:
        fp.write(chunk)
        adapter.write(chunk)

        #Repeat for the commands
        chunk = da_device_chunk(config, lst, "command")

        #Put the chunk in the files:
        fp.write(chunk)
        adapter.write(chunk)

        adapter.write("</adapter>\n")
        fp.write("</adapter>\n")
        fp.write("</root>\n")
        fp.close()
        i += 1
    adapter.write("</root>\n")
    adapter.close()

if __name__ == "__main__":
    main()
