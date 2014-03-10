class ParserError(Exception):
    pass

def parse_tabbed_config(conf_file):
    """
    Given a input file conf_file this will split it into tokens by line,
    ignoring comments.
    
    yields the split tokens until EOF
    """
    f = file(conf_file)
    result = []
    for line in f:
        do = line.split("#")[0]
        tokens = do.split()
        yield tokens

def get_fabric_hosts(conf_file):
    """
    Given an input file conf_file this will spit it into tokens by line.

    It will check to make sure there are 3 tokens. If there are not
    3 tokens then a Parser Error is raised.

    If there are more than 4 tokens, then the remaining tokens are rejoined
    and then split by commas (These are options for installing things)
    
    returns a list of dictionaries which are guaranteed to contain the keys
    hostname, user, ip and options.
    """
    for tokens in parse_tabbed_config(conf_file)
        try:
            d = {
                'hostname': tokens[0],
                'user':     tokens[1],
                'ip':       tokens[2],
            }
        except IndexError:
            raise ParserError("Couldn't parse '{}', a token is missing.".format(d))
        if len(tokens) >= 3:
            rem = " ".join(tokens[3:])
            d['options'] = rem.split(",")
        else:
            d['options'] = []
        result.append(d)
    return result


