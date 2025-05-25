# Errors:
- Setting all voxels to occupied literally fries my GPU and i have to restart the computer to recover. wtf? all chunks still have at minimum 1 face (wh 2 thf each chunk borders void)? NVM this also happens when doing %2 population. HUH? maybe VRAM full ? maybe due to having a video playing in background and running on an intergrated GPU? Honestly this is not high priority.

# TODO:
- Look into building a region system, with each region containing a static array of indices into a chunk vector. Then the world would contain a vector of regions, and when finding chunk given position we can first find region (int div region_size (in chunk scale)), linear search region vector (low cost, since vector should only be ~4-8 items max), then use chunk map in region to find chunk. Issues:
    - Would have to update region map whenever chunks are removed from chunk vector
    - Would make adding chunks way more expensive - maybe not an issue with server - client threading? to test
    - World saving (not yet in plan) would need to keep track of vector orders of chunks, or rebuild region maps (probably best)

- Alternative would be a global world chunk hashmap by chunk positions, but this would be a lot more work
