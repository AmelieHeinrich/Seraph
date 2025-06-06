import os
import shutil

# Set the directory containing test images
test_dir = "Data/Tests"

# Make sure the directory exists
if not os.path.isdir(test_dir):
    print(f"Directory not found: {test_dir}")
    exit(1)

# Iterate through files in the directory
for filename in os.listdir(test_dir):
    if filename.endswith("D3D12.png") and filename.find("Magma") == -1:
        src_path = os.path.join(test_dir, filename)
        
        # Replace 'D3D12' with 'Golden'
        golden_filename = filename.replace("D3D12", "Golden")
        dst_path = "Golden" + os.path.join(test_dir, golden_filename)
        
        # Copy file
        shutil.copyfile(src_path, dst_path)
        print(f"Copied: {src_path} -> {dst_path}")
