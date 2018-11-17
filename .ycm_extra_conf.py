def Settings(**kwargs):
    return {
        'flags' : [ '-Wall', '-Wextra', '-Werror', '-pedantic', '-DTOOL', '-I.', '-I3p/atto', '-I3p', '-O0', '-g', '-std=c++17' ]
    }
