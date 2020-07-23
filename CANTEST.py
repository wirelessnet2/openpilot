timestamps = [69, 5, 1, 2, 1, 1, 1, 5, 1, 3, 1, 5, 1, 2, 1, 5, 5, 1, 5, 1, 2, 1]

def calculateBitbang(timestamps):
    bitstuff = False
    i = 1
    bit = 0
    msg = []
    while i < len(timestamps):
        if timestamps[i] == 5:
            if timestamps[i+1] == 1:
                if bitstuff:
                    msg.extend([bit] * (4 + timestamps[i+2]))
                    if timestamps[i+2] == 5 and timestamps[i+3] == 1:
                        i += 3
                        bitstuff = False
                    else: i += 2
                else:
                    msg.extend([bit] * (5 + timestamps[i+2]))
                    i += 2
            else:
                msg.extend([bit] * 5)
                bitstuff = True
        else:
            if bitstuff:
                msg.extend([bit] * (timestamps[i]-1))
                bitstuff = False
            else: msg.extend([bit] * (timestamps[i]))
        i += 1
        if bit == 0: bit = 1
        else: bit = 0
    return msg

print(calculateBitbang(timestamps))
