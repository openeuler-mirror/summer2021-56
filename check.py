import os
import subprocess
file_content = []
error_log=[]
os.chdir('./linux-next')


def delete_a_include(filename, include):
    global file_content
    s=''
    with open(filename, 'r') as f:
        file_content = f.readlines()
    with open(filename, 'w') as f:
        for i in file_content:
            if i.startswith('#include') and i.find(include) != -1:
                s=i
                continue
            f.write(i)
    return s


def test_compile(filename, module, include):
    global file_content
    global error_log
    result = os.system('make '+module+'/ -j12')
    if(result != 0):
        with open(filename, 'w') as f:
            f.writelines(file_content)
        error_log.append((filename,include))
        result = os.system('make '+module+' -j12')
        if(result != 0):
            print(filename+' can not be compiled')
            exit()
        return False
    if include==""or include==" ":
        return False
    try:
        file_content.remove(include)
    finally:
        with open("./error.txt",'w') as f:
            for i in error_log:
                f.write(i[0])
                f.write('   ')
                f.write(i[1])
                f.write('\n----------------------\n')
    return True


a = subprocess.call('g++ ../project.cpp -std=c++17 -o check -O2', shell=True)
b = subprocess.call('make allyesconfig', shell=True)
if a != 0 or b != 0:
    print('error')
    exit()
patch_num = 0
file_remove_table = {}
module = ['./net']
for i in module:
    os.system('make '+i+'/ -j12')
    #if(not os.path.exists(i+'.txt')):
    os.system('./check -il ./ -o '+i +
                  '.txt'+' -move ./'+i+'/')
    with open(i+'.txt', 'r') as f:
        l = f.readlines()
        for j in l:
            two_part = j.split(' ')
            if two_part[0]=='' or two_part[1]=='' or two_part[0].find('errno.h')!=-1 or two_part[0].find('types.h')!=-1 or two_part[0].find('unaligned.h')!=-1 or two_part[0].find('init.h')!=-1 or two_part[0].find('kernel.h')!=-1 or two_part[0].find('module.h')!=-1:
                continue 
            if two_part[1] in file_remove_table:
                file_remove_table[two_part[1]].append(two_part[0])
            else:
                file_remove_table[two_part[1]] = [two_part[0]]
    for filename, include_list in file_remove_table.items():
        filename = filename.strip()
        if patch_num > 200000:
            patch_num=0
            break
        true_delete = []
        if len(include_list)>4:
            continue
        for k in include_list:
            s=delete_a_include(filename, k)
            if test_compile(filename, i, s) == True:
                true_delete.append(k)
        if len(true_delete) > 0:
            with open('patch.txt', 'w') as f:
                f.write(filename[filename.find(i)+2:])
                f.write(':')
                f.write(' remove superfluous headers')
                name = filename[filename.rfind('/')+1:]
                f.write('\n\n')
                s = name+' hasn\'t use any macro or function declared in '
                count = 65
                for m in range(len(true_delete)-1):
                    if len(s) > count:
                        s += '\n'
                        count += 65
                    s = s+true_delete[m]
                    if m != len(true_delete)-2:
                        s = s+', '
                if len(true_delete)>1:
                    s += ' and '+true_delete[len(true_delete)-1]+'.\n'
                else:
                    s+=true_delete[0]+'.\n'
                f.write(s)
                f.write('Thus, these files can be removed from ')
                f.write(name)
                f.write(' safely without\n')
                f.write('affecting the compilation of the '+i+' module')
            result = os.system('git add '+filename)
            if(result != 0):
                print('git add failure')
            os.system('git commit -F ./patch.txt')
            os.system('git format-patch -1')
            true_delete = []
            count = 70
            patch_num += 1
with open("./error.txt",'w') as f:
    for i in error_log:
        f.write(i[0])
        f.write('   ')
        f.write(i[1])
        f.write('\n----------------------\n')