
def string_to_c_string(string):
    return string.encode('utf-8')

def strings_to_c_strings(strings):
    
    result = []
    for string in strings:
        result.append(string_to_c_string(string))

    return result
