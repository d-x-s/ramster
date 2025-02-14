# M1 Submission Notes
This write-up will be moved elsewhere upon the actual M1 submission.

## Box2D Prerequisites
- To get started with Box2D, read the [documentation](https://box2d.org/documentation/).
- Understand how the world and bodies interact [HelloBox2D](https://box2d.org/documentation/hello.html).
- Read the [Box2D FAQ](https://box2d.org/documentation/md_faq.html).
- One of the best references is here [box2d-raylib](https://github.com/erincatto/box2d-raylib)

## PR Etiquette
- Follow a good branch naming convention: `<milestone>-<name>-<feature-name>`.  
  Example: `m1-davis-map_system`
- Include a written description of your changes.
- Include a GIF or screenshots of your contribution.
- PRs must have **one reviewer**.  
  - In addition to reviewing code, the reviewer **must** check out the branch, build, and playtest the changes.

## Building
- Check that you are on VS2022
- FetchContent is used to get the Box2D library so make sure you have an internet connection
- in root, do <mkdir build>, <cd build>, <Cmake ..>
- <cd build> and open ramster.sln
- build the Ramster project
- exe is in build/Debug