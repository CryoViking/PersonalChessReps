const button = requireElement<HTMLButtonElement>("#ping-button");
const secondButton = requireElement<HTMLButtonElement>("#fail-ping-button");
const result = requireElement<HTMLParagraphElement>("#result");

function requireElement<T extends HTMLElement>(id: string) {
  const element = document.querySelector<T>(id);
  if (element === null) throw new Error(`missing ${id}`);
  return element;
}

button.addEventListener("click", async () => {
  await callPing("e4");
});

secondButton.addEventListener("click", async () => {
  await callPing("error");
});

async function callPing(message: string) {
  try {
    const reply = await window.ping(message);
    if (reply.ok) {
      result.textContent = `C++ replied with success: ${reply.value}`;
    } else {
      result.textContent = `C++ replied with error: ${reply.error}`;
    }
  } catch (error) {
    result.textContent = `Ping failed: ${error}`;
  }
}
