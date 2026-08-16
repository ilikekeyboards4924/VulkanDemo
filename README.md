## disclaimer
currently when the program selects a gpu to use, it selects the first discrete gpu it finds.
this means that this program may not work on laptops, or systems without a dedicated gpu.
## todo
- add separate semaphores for each swapchain image. right now all the swapchain images are using the same 2 semaphores.