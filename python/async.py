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

import time
import asyncio

done = False

async def foo(bar):
  done = True
  return "done with foo"

async def blah():
  done = True
  return "done with blah"

async def a_main():
  task = asyncio.ensure_future(blah())
  print("In main")
  await task

loop.run_until_complete(a_main())
for x in range(100):
  print(done)
  time.sleep(1)
