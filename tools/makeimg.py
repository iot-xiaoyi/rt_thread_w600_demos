# -*- coding: utf-8 -*-  

import os
import sys
import shutil
import subprocess
import time
import platform

wm_librarie_path = '../../libraries/WM_Libraries'
out_path = './Bin'

debug_info = False

bin_file = './rtthread.bin'

version_file = wm_librarie_path + '/rtthread/version.txt'
secboot_file = wm_librarie_path + '/rtthread/secboot.img'

# wm_gzip_file = wm_librarie_path + '/Tools/wm_gzip.py'
# makeimg_file = wm_librarie_path + '/Tools/makeimg.py'
# makeimg_all_file = wm_librarie_path + '/Tools/makeimg_fls.py'

wm_gzip_file = wm_librarie_path + '/Tools/wm_gzip.exe'
makeimg_file = wm_librarie_path + '/Tools/makeimg.exe'
makeimg_all_file = wm_librarie_path + '/Tools/makeimg_all.exe'

def execute_command(cmdstring, cwd=None, shell=True):
    """Execute the system command at the specified address."""

    if shell:
        cmdstring_list = cmdstring

    sub = subprocess.Popen(cmdstring_list, cwd=cwd, stdin=subprocess.PIPE,
                           stdout=subprocess.PIPE, shell=shell, bufsize=8192)

    stdout_str = ""
    while sub.poll() is None:
        stdout_str += str(sub.stdout.read())
        time.sleep(0.1)

    return stdout_str

def copy_file(name, path):
    res = True
    if os.path.exists(path):
        shutil.copy(path, out_path)
    else:
        print('makeimg err! No ' + name + ' file found: ' + path)
        res = False
    return res

def is_exists(name, path):
    res = True
    if not os.path.exists(path):
        print('makeimg err! No ' + name + ' file found: ' + path)
        res = False
    return res

def get_exec_path(path):
    (file_path, file_name) = os.path.split(path)
    (name, extend) = os.path.splitext(file_name)

    exec_path = ''
    if (platform.system() == "Windows"):
        exec_path = os.path.abspath(file_path + '/' + name + '.exe')
    elif (platform.system() == "Linux"):
        exec_path = os.path.abspath(file_path + '/' + name)

    if debug_info:
        print('file_path: ' + file_path)
        print('file_name: ' + file_name)
        print('name: ' + name)
        print('extend: ' + extend)

    return exec_path

def do_makeimg(tool_path, param):
    str = "\"" + tool_path +  "\"" + ' ' + param
    if debug_info:
        print('exec cmd: ' + str);

    execute_command(str)

if __name__=='__main__':
    if not os.path.exists(out_path): os.mkdir(out_path)
    wm_gzip_file = get_exec_path(wm_gzip_file)
    makeimg_file = get_exec_path(makeimg_file)
    makeimg_all_file = get_exec_path(makeimg_all_file)
    if not copy_file('bin', bin_file): exit(0)
    if not copy_file('version', version_file): exit(0)
    if not copy_file('secboot', secboot_file): exit(0)
    if not is_exists('wm_gzip', wm_gzip_file): exit(0)
    if not is_exists('makeimg', makeimg_file): exit(0)
    if not is_exists('makeimg_all', makeimg_all_file): exit(0)
    (bin_file_path,bin_file_name) = os.path.split(bin_file)
    (bin_name,bin_extend) = os.path.splitext(bin_file_name)
    (version_file_path,version_file_name) = os.path.split(version_file)
    (secboot_file_path,secboot_file_name) = os.path.split(secboot_file)
    if debug_info: print('bin_file_name:' + bin_file_name + 'bin_name:' + bin_name + 'bin_extend:' + bin_extend + 'version_file_name:' + version_file_name + 'secboot_file_name:' + secboot_file_name)
    print('makeimg...')

    gzip_param = "\"" + out_path + '/' + bin_file_name + "\""
    make_img_param = "\"" + out_path + '/' + bin_file_name + "\"" + ' ' + "\"" + out_path + '/' + bin_name + '.img' + "\"" + ' 0' + ' 0' + ' ' + "\"" + out_path + '/' + version_file_name + "\"" + ' 90000' + ' 10100'
    make_GZ_param = "\"" + out_path + '/' + bin_file_name + '.gz' + "\"" + ' ' + "\"" + out_path + '/' + bin_name + '_GZ' + '.img' +"\"" + ' 0' + ' 1' + ' ' + "\"" + out_path + '/' + version_file_name + "\"" + ' 90000' + ' 10100' + ' ' + "\"" + out_path + '/' + bin_file_name + "\""
    make_SEC_param = "\"" + out_path + '/' + bin_file_name + "\"" + ' ' + "\"" + out_path + '/' + bin_file_name + '_SEC.img' + "\"" + ' 0' + ' 0' + ' ' + "\"" + out_path + '/' + version_file_name + "\"" + ' 90000' + ' 10100'
    make_FLS_param = "\"" + out_path + '/' + secboot_file_name + "\"" + ' ' + "\"" + out_path + '/' + bin_name + '.img' + "\"" + ' ' + "\"" + out_path + '/' + bin_name + '.FLS' + "\""

    if debug_info:
        print('gzip_param' + gzip_param)
        print('make_img_param' + make_img_param)
        print('make_GZ_param' + make_GZ_param)
        print('make_SEC_param' + make_SEC_param)
        print('make_FLS_param' + make_FLS_param)

    print("make gzip")
    do_makeimg(wm_gzip_file, gzip_param)
    print("make img")
    do_makeimg(makeimg_file, make_img_param)
    print("make GZ")
    do_makeimg(makeimg_file, make_GZ_param)
    print("make SEC")
    do_makeimg(makeimg_file, make_SEC_param)
    print("make FLS")
    do_makeimg(makeimg_all_file, make_FLS_param)
    print('end')