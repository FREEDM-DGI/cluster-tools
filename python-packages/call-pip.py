#This script simply facilitates calling fabric when it is installed as a user
#rather than as root. We do this because the command pip may not be on the path

import pip

pip.main()
