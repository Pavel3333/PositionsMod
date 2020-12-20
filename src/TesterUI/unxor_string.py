a = raw_input().split('\\x')
a = filter(bool, a)
b = []
for i in a:
    b.append(('0' + i)[-2:])

for i in xrange(len(b) - 2, 0, -1):
    b[i] = str(hex(int(b[i], 16) ^ int(b[i - 1], 16)))

c_hex = ''
c_whex = ''

for i in b[:-1]:
    i_whex = i.split('0x')[-1]
    
    c_hex += '\\x'[-2:] + i_whex
    c_whex += chr(int(i_whex, 16))

print 'hex        : "' + c_hex + '"'
print 'without hex: "' + c_whex + '"'

    
