# import concurrent.futures
# import time

# done = False

# def blah():
#   time.sleep(5)
#   return "done with blah"

# def foo(bar):
#   time.sleep(10)
#   return "Returned value: {}".format(bar)

# def cb():
#   done = True

# executor = concurrent.futures.ThreadPoolExecutor()
# # thread = executor.submit(foo, args=('done!'))
# thread = executor.submit(blah)
# print(thread.result())
# # thread.add_done_callback(cb)

# while thread.running():
#   print(done)
#   time.sleep(1)

# print("Thread is no longer running, global is now {}".format(done))

# import time
# import asyncio

# done = False

# async def foo(bar):
#   done = True
#   return "foo with arg {}".format(bar)

# async def blah():
#   done = True
#   return "blah"

# async def a_main():
#   task = asyncio.ensure_future(blah())
#   print("In main")
#   await task

# loop.run_until_complete(a_main())
# for x in range(100):
#   print(done)
#   time.sleep(1)

from multiprocessing import Pool, freeze_support
import time
import os
done = False

def foo(bar):
  time.sleep(5)
  return "foo with arg {}".format(bar)

def blah():
  time.sleep(5)
  return "blah"

def cb():
  done = True

def call_foo():
  pool = Pool(processes=1)
  result = pool.apply_async(foo, ('bar',))
  # result = pool.apply_async(blah)
  timestamp = time.time()
  while not result.ready():
    os.system('clear')
    print("Not done yet, timer is {}".format(int(time.time() - timestamp) + 1))
    time.sleep(1)

  # pool.close()
  return result

def main():
  result = call_foo()
  result = call_foo()

  print("Done! Return value is {}".format(result.get()))

if __name__ == '__main__':
  freeze_support()
  main()

# import threading
# import time
# import os

# done = False

# def foo(bar):
#   time.sleep(5)
#   bar = True

# thread = threading.Thread(target=foo, args=(done,))
# thread.start()

# timestamp = time.time()

# while thread.is_alive():
#   os.system('clear')
#   print("Not done yet, timer is {}".format(int(time.time() - timestamp) + 1))
#   time.sleep(1)

# print("Done! Semaphore is {}".format(done))
