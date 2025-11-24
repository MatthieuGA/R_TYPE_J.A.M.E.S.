---
name: Task
about: Create a task to implement the project
title: New task
labels: enhancement
assignees: ''

---

## Objective <!-- Mandatory -->
<!--
    Here you details what are the objectives of this task

    Example:
    Implement a buffer that will be used to write responses to clients. Instead of writing to clients directly, we will add the response to this buffer that will write to the clients buffer every ticks
-->

## Context <!-- Optional -->
<!--
    Here you can give context to better understand the task

    Example:
    For context, currently, when the server wants to send a message to a client, it directly sends it without checking if the writing was successful
-->

## Steps <!-- Mandatory -->
<!--
    Here you list steps by steps how to do this task

    Example:
    - [ ] Create a buffer
    - [ ] Replace the "writes" by "add to writing buffer"
    - [ ] Every ticks, parse the writing buffer and write to clients
-->

## Acceptance Criteria <!-- Mandatory -->
<!--
    Here you list the criteria to tell whether a task is finished or not

    Example:
    - [ ] The server doesn't direcly write a message to a client
    - [ ] The server stores the raw buffer into an array to parse it and interpret it later
    - [ ] Every ticks, the server empty the array to write all the messages stored to their clients
-->

## Relevant Links <!-- Optional -->
<!--
    Here you can give links for even more context

    Example:
    - https://cboard.cprogramming.com/c-programming/178183-c-stdin-buffer.html
-->
