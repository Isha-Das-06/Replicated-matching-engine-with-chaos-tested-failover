import csv
import random
import sys

def generate(num_orders, filename):
    with open(filename, 'w', newline='') as f:
        writer = csv.writer(f)
        # Format: Time, Event Type, Order ID, Size, Price, Direction
        # Direction: 1 (Buy), -1 (Sell)
        # Event Type: 1 (Submission)
        
        current_time = 34200.0 # 9:30 AM in seconds
        current_id = 1
        
        for i in range(num_orders):
            direction = random.choice([1, -1])
            # Prices around $10 (1000 cents)
            if direction == 1:
                price = random.randint(990, 1000)
            else:
                price = random.randint(1000, 1010)
            
            size = random.randint(10, 500)
            
            writer.writerow([
                f"{current_time:.9f}",
                1, # Submission
                current_id,
                size,
                price,
                direction
            ])
            
            current_time += 0.0001
            current_id += 1
            
if __name__ == "__main__":
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} <num_orders> <output_csv>")
        sys.exit(1)
    
    num_orders = int(sys.argv[1])
    filename = sys.argv[2]
    generate(num_orders, filename)
    print(f"Generated {num_orders} dummy LOBSTER messages to {filename}")
