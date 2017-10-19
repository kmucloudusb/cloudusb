import os

Kbyte = 1024

def replace_cache():
    st = os.statvfs('/')
    du = (st.f_bavail * st.f_frsize) / Kbyte
    print(du)

replace_cache()