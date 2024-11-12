import requests
import concurrent.futures
import time

# Configuration
url = "http://localhost:8080/script.py"  # Your server endpoint
num_requests = 1000                      # Total number of requests to send
concurrent_workers = 50                  # Number of threads to use

def send_request():
    try:
        response = requests.get(url)
        return response.status_code
    except requests.exceptions.RequestException as e:
        print(f"Request failed: {e}")
        return None

start_time = time.time()

# Run the requests concurrently
with concurrent.futures.ThreadPoolExecutor(max_workers=concurrent_workers) as executor:
    futures = [executor.submit(send_request) for _ in range(num_requests)]
    results = [future.result() for future in concurrent.futures.as_completed(futures)]

end_time = time.time()

# Results
successful_requests = sum(1 for result in results if result == 200)
failed_requests = sum(1 for result in results if result is None)
total_time = end_time - start_time

print(f"Total requests: {num_requests}")
print(f"Successful requests: {successful_requests}")
print(f"Failed requests: {failed_requests}")
print(f"Total time taken: {total_time:.2f} seconds")
print(f"Requests per second: {num_requests / total_time:.2f}")