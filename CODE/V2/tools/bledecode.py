def decode_ble_bytes(data_hex):
    # Convert hex string to bytes
    data_bytes = bytes.fromhex(data_hex)
    
    print(f"Raw data: {data_hex} → {data_bytes.hex()}")
    print("\nByte breakdown:")
    for i, byte in enumerate(data_bytes):
        signed_byte = int.from_bytes([byte], byteorder='big', signed=True)
        print(f"Byte {i}: 0x{byte:02X} ({signed_byte})")
    
    # Protocol structure analysis
    header = data_bytes[0]
    data = data_bytes[1:-2]  # Data bytes (excluding header, checksum, footer)
    checksum_received = data_bytes[-2]
    footer = data_bytes[-1]
    
    # Calculate expected checksum (using unsigned bytes)
    checksum_calculated = sum(data) & 0xFF
    
    print("\nProtocol analysis:")
    print(f"Header: 0x{header:02X} (expected 0xA5)")
    print(f"Data bytes (signed):")
    for i, byte in enumerate(data):
        signed_byte = int.from_bytes([byte], byteorder='big', signed=True)
        print(f"  Byte {i}: 0x{byte:02X} ({signed_byte})")
    print(f"Checksum received: 0x{checksum_received:02X}")
    print(f"Checksum calculated: 0x{checksum_calculated:02X}")
    print(f"Footer: 0x{footer:02X} (expected 0x5A)")

def decode_ble_ints(hex_str, num_integers):
    """
    Decode BLE packet with configurable integer count
    Returns: (is_valid, [decoded_values])
    """
    try:
        data = bytes.fromhex(hex_str)
    except ValueError:
        return False, []
    
    # Basic validation
    if len(data) < 4 or data[0] != 0xA5 or data[-1] != 0x5A:
        return False, []
    
    # Extract components
    header = data[0]
    data_bytes = data[1:-2]
    received_checksum = data[-2]
    footer = data[-1]
    
    # Calculate expected values
    expected_length = 4 * num_integers + 3  # Header + data + checksum + footer
    calc_checksum = sum(data_bytes) & 0xFF
    
    # Validate structure
    valid = (
        len(data) == expected_length and
        header == 0xA5 and
        footer == 0x5A and
        calc_checksum == received_checksum
    )
    
    # Decode integers (big-endian signed)
    integers = []
    for i in range(0, 4 * num_integers, 4):
        chunk = data_bytes[i:i+4]
        if len(chunk) != 4:
            break
        integers.append(int.from_bytes(chunk, byteorder='little', signed=True))
    
    return valid, integers

# Example usage with your packet
hex_str = "A5000001F4FFFFFE0CFD5A"  # Contains 2 integers
num_values = 2

is_valid, values = decode_ble_ints(hex_str, num_values)

print(f"Packet Valid: {is_valid}")
print(f"Decoded {len(values)}/{num_values} integers:")
for i, val in enumerate(values):
    print(f"  Integer {i+1}: {val} (0x{val:08X})")

# You can add new packets like this:
# new_data = "A57F7F817F5A"  # Example from your comment
# is_valid, values = decode_ble_packet(new_data, num_values=1)


# Example usage
#decode_ble_bytes("A500000F6900000F6AF15A")

# A5 7F 7F 81 7F 5A
#   │   │   │   │
#   │   │   │   └── Footer
#   │   │   └── Checksum (sum of 0x7F + 0x7F + 0x81 = 0x17F → 0x7F)
#   └── Data Bytes (3 bytes)