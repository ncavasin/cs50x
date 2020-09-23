from cs50 import get_int
while True:
    height = get_int("Height: ")
    if height in range(1, 9, 1):
        break

for i in range(0, height):
    print(" " * (height - (i+1)) + "#" * (i+1) + "  " + "#" * (i+1))
