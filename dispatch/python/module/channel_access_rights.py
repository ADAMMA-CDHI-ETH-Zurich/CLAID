from enum import Enum 

class ChannelAccessRights(Enum):
    NONE = 0
    WRITE = 1
    READ = 2
    READ_WRITE = 3