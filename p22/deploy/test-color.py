#  c:\WINDOWS\system32\CONFIG.NT
# add device=c:\windows\system32\ansi.sys

# http://pypi.python.org/pypi/termcolor
# http://pypi.python.org/packages/source/t/termcolor/termcolor-0.1.2.tar.gz#md5=bc0f9923c8c82643a6b48e1b0f87399f

# http://pypi.python.org/pypi/colorama

class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'

    def disable(self):
        self.HEADER = ''
        self.OKBLUE = ''
        self.OKGREEN = ''
        self.WARNING = ''
        self.FAIL = ''
        self.ENDC = ''

print bcolors.WARNING + "Warning: No active frommets remain. Continue?" + bcolors.ENDC

from termcolor import colored

print colored('Hello, World!', 'red', attrs=['reverse', 'blink'])
print colored('Hello, World!', 'green', 'on_red')

red_on_cyan = lambda x: colored(x, 'red', 'on_cyan')
print red_on_cyan('Hello, World!')
print red_on_cyan('Hello, Universe!')

from colorama import init
init(autoreset=True)

from colorama import Fore, Back, Style
print Fore.RED + 'some red text'
print Back.GREEN + 'and with a green background'
print Style.DIM + 'and in dim text'
print 'back to normal now'
